/*
 * Copyright © 2016 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *
 */
#include <drm/drm_edid.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_dp_dual_mode_helper.h>
#include "intel_drv.h"

/* LSPCON OUI Vendor ID(signatures) */
#define LSPCON_VENDOR_PARADE_OUI 0x001CF8
#define LSPCON_VENDOR_MCA_OUI 0x0060AD

/* AUX addresses to write MCA AVI IF */
#define LSPCON_MCA_AVI_IF_WRITE_OFFSET 0x5C0
#define LSPCON_MCA_AVI_IF_CTRL 0x5DF
#define  LSPCON_MCA_AVI_IF_KICKOFF (1 << 0)
#define  LSPCON_MCA_AVI_IF_HANDLED (1 << 1)

static struct intel_dp *lspcon_to_intel_dp(struct intel_lspcon *lspcon)
{
	struct intel_digital_port *dig_port =
		container_of(lspcon, struct intel_digital_port, lspcon);

	return &dig_port->dp;
}

static const char *lspcon_mode_name(enum drm_lspcon_mode mode)
{
	switch (mode) {
	case DRM_LSPCON_MODE_PCON:
		return "PCON";
	case DRM_LSPCON_MODE_LS:
		return "LS";
	case DRM_LSPCON_MODE_INVALID:
		return "INVALID";
	default:
		MISSING_CASE(mode);
		return "INVALID";
	}
}

static bool lspcon_detect_vendor(struct intel_lspcon *lspcon)
{
	struct intel_dp *dp = lspcon_to_intel_dp(lspcon);
	struct drm_dp_dpcd_ident *ident;
	u32 vendor_oui;

	if (drm_dp_read_desc(&dp->aux, &dp->desc, drm_dp_is_branch(dp->dpcd))) {
		DRM_ERROR("Can't read description\n");
		return false;
	}

	ident = &dp->desc.ident;
	vendor_oui = (ident->oui[0] << 16) | (ident->oui[1] << 8) |
		      ident->oui[2];

	switch (vendor_oui) {
	case LSPCON_VENDOR_MCA_OUI:
		lspcon->vendor = LSPCON_VENDOR_MCA;
		DRM_DEBUG_KMS("Vendor: Mega Chips\n");
		break;

	case LSPCON_VENDOR_PARADE_OUI:
		lspcon->vendor = LSPCON_VENDOR_PARADE;
		DRM_DEBUG_KMS("Vendor: Parade Tech\n");
		break;

	default:
		DRM_ERROR("Invalid/Unknown vendor OUI\n");
		return false;
	}

	return true;
}

static enum drm_lspcon_mode lspcon_get_current_mode(struct intel_lspcon *lspcon)
{
	enum drm_lspcon_mode current_mode;
	struct i2c_adapter *adapter = &lspcon_to_intel_dp(lspcon)->aux.ddc;

	if (drm_lspcon_get_mode(adapter, &current_mode)) {
		DRM_DEBUG_KMS("Error reading LSPCON mode\n");
		return DRM_LSPCON_MODE_INVALID;
	}
	return current_mode;
}

static enum drm_lspcon_mode lspcon_wait_mode(struct intel_lspcon *lspcon,
					     enum drm_lspcon_mode mode)
{
	enum drm_lspcon_mode current_mode;

	current_mode = lspcon_get_current_mode(lspcon);
	if (current_mode == mode)
		goto out;

	DRM_DEBUG_KMS("Waiting for LSPCON mode %s to settle\n",
		      lspcon_mode_name(mode));

	wait_for((current_mode = lspcon_get_current_mode(lspcon)) == mode, 400);
	if (current_mode != mode)
		DRM_ERROR("LSPCON mode hasn't settled\n");

out:
	DRM_DEBUG_KMS("Current LSPCON mode %s\n",
		      lspcon_mode_name(current_mode));

	return current_mode;
}

static int lspcon_change_mode(struct intel_lspcon *lspcon,
			      enum drm_lspcon_mode mode)
{
	int err;
	enum drm_lspcon_mode current_mode;
	struct i2c_adapter *adapter = &lspcon_to_intel_dp(lspcon)->aux.ddc;

	err = drm_lspcon_get_mode(adapter, &current_mode);
	if (err) {
		DRM_ERROR("Error reading LSPCON mode\n");
		return err;
	}

	if (current_mode == mode) {
		DRM_DEBUG_KMS("Current mode = desired LSPCON mode\n");
		return 0;
	}

	err = drm_lspcon_set_mode(adapter, mode);
	if (err < 0) {
		DRM_ERROR("LSPCON mode change failed\n");
		return err;
	}

	lspcon->mode = mode;
	DRM_DEBUG_KMS("LSPCON mode changed done\n");
	return 0;
}

static bool lspcon_wake_native_aux_ch(struct intel_lspcon *lspcon)
{
	u8 rev;

	if (drm_dp_dpcd_readb(&lspcon_to_intel_dp(lspcon)->aux, DP_DPCD_REV,
			      &rev) != 1) {
		DRM_DEBUG_KMS("Native AUX CH down\n");
		return false;
	}

	DRM_DEBUG_KMS("Native AUX CH up, DPCD version: %d.%d\n",
		      rev >> 4, rev & 0xf);

	return true;
}

static bool lspcon_probe(struct intel_lspcon *lspcon)
{
	int retry;
	enum drm_dp_dual_mode_type adaptor_type;
	struct i2c_adapter *adapter = &lspcon_to_intel_dp(lspcon)->aux.ddc;
	enum drm_lspcon_mode expected_mode;

	expected_mode = lspcon_wake_native_aux_ch(lspcon) ?
			DRM_LSPCON_MODE_PCON : DRM_LSPCON_MODE_LS;

	/* Lets probe the adaptor and check its type */
	for (retry = 0; retry < 6; retry++) {
		if (retry)
			usleep_range(500, 1000);

		adaptor_type = drm_dp_dual_mode_detect(adapter);
		if (adaptor_type == DRM_DP_DUAL_MODE_LSPCON)
			break;
	}

	if (adaptor_type != DRM_DP_DUAL_MODE_LSPCON) {
		DRM_DEBUG_KMS("No LSPCON detected, found %s\n",
			       drm_dp_get_dual_mode_type_name(adaptor_type));
		return false;
	}

	/* Yay ... got a LSPCON device */
	DRM_DEBUG_KMS("LSPCON detected\n");
	lspcon->mode = lspcon_wait_mode(lspcon, expected_mode);

	/*
	 * In the SW state machine, lets Put LSPCON in PCON mode only.
	 * In this way, it will work with both HDMI 1.4 sinks as well as HDMI
	 * 2.0 sinks.
	 */
	if (lspcon->active && lspcon->mode != DRM_LSPCON_MODE_PCON) {
		if (lspcon_change_mode(lspcon, DRM_LSPCON_MODE_PCON) < 0) {
			DRM_ERROR("LSPCON mode change to PCON failed\n");
			return false;
		}
	}
	return true;
}

static void lspcon_resume_in_pcon_wa(struct intel_lspcon *lspcon)
{
	struct intel_dp *intel_dp = lspcon_to_intel_dp(lspcon);
	struct intel_digital_port *dig_port = dp_to_dig_port(intel_dp);
	unsigned long start = jiffies;

	while (1) {
		if (intel_digital_port_connected(&dig_port->base)) {
			DRM_DEBUG_KMS("LSPCON recovering in PCON mode after %u ms\n",
				      jiffies_to_msecs(jiffies - start));
			return;
		}

		if (time_after(jiffies, start + msecs_to_jiffies(1000)))
			break;

		usleep_range(10000, 15000);
	}

	DRM_DEBUG_KMS("LSPCON DP descriptor mismatch after resume\n");
}

static bool _lspcon_write_avi_infoframe_mca(struct drm_dp_aux *aux,
					    const uint8_t *buffer, ssize_t len)
{
	int ret;
	uint32_t val = 0;
	uint32_t retry;
	uint16_t reg;
	const uint8_t *data = buffer;

	reg = LSPCON_MCA_AVI_IF_WRITE_OFFSET;
	while (val < len) {
		/* DPCD write for AVI IF can fail on a slow FW day, so retry */
		for (retry = 0; retry < 5; retry++) {
			ret = drm_dp_dpcd_write(aux, reg, (void *)data, 1);
			if (ret == 1) {
				break;
			} else if (retry < 4) {
				mdelay(50);
				continue;
			} else {
				DRM_ERROR("DPCD write failed at:0x%x\n", reg);
				return false;
			}
		}
		val++; reg++; data++;
	}

	val = 0;
	reg = LSPCON_MCA_AVI_IF_CTRL;
	ret = drm_dp_dpcd_read(aux, reg, &val, 1);
	if (ret < 0) {
		DRM_ERROR("DPCD read failed, address 0x%x\n", reg);
		return false;
	}

	/* Indicate LSPCON chip about infoframe, clear bit 1 and set bit 0 */
	val &= ~LSPCON_MCA_AVI_IF_HANDLED;
	val |= LSPCON_MCA_AVI_IF_KICKOFF;

	ret = drm_dp_dpcd_write(aux, reg, &val, 1);
	if (ret < 0) {
		DRM_ERROR("DPCD read failed, address 0x%x\n", reg);
		return false;
	}

	val = 0;
	ret = drm_dp_dpcd_read(aux, reg, &val, 1);
	if (ret < 0) {
		DRM_ERROR("DPCD read failed, address 0x%x\n", reg);
		return false;
	}

	if (val == LSPCON_MCA_AVI_IF_HANDLED)
		DRM_DEBUG_KMS("AVI IF handled by FW\n");

	return true;
}

void lspcon_write_infoframe(struct intel_encoder *encoder,
			    const struct intel_crtc_state *crtc_state,
			    unsigned int type,
			    const void *frame, ssize_t len)
{
	bool ret = true;
	struct intel_dp *intel_dp = enc_to_intel_dp(&encoder->base);
	struct intel_lspcon *lspcon = enc_to_intel_lspcon(&encoder->base);

	/* LSPCON only needs AVI IF */
	if (type != HDMI_INFOFRAME_TYPE_AVI)
		return;

	if (lspcon->vendor == LSPCON_VENDOR_MCA)
		ret = _lspcon_write_avi_infoframe_mca(&intel_dp->aux,
						      frame, len);
	if (!ret) {
		DRM_ERROR("Failed to write AVI infoframes\n");
		return;
	}

	DRM_DEBUG_DRIVER("AVI infoframes updated successfully\n");
}

void lspcon_set_infoframes(struct intel_encoder *encoder,
			   bool enable,
			   const struct intel_crtc_state *crtc_state,
			   const struct drm_connector_state *conn_state)
{
	ssize_t ret;
	union hdmi_infoframe frame;
	uint8_t buf[VIDEO_DIP_DATA_SIZE];
	struct intel_digital_port *dig_port = enc_to_dig_port(&encoder->base);
	struct intel_lspcon *lspcon = &dig_port->lspcon;
	struct intel_dp *intel_dp = &dig_port->dp;
	struct drm_connector *connector = &intel_dp->attached_connector->base;
	const struct drm_display_mode *mode = &crtc_state->base.adjusted_mode;
	bool is_hdmi2_sink = connector->display_info.hdmi.scdc.supported;

	if (!lspcon->active) {
		DRM_ERROR("Writing infoframes while LSPCON disabled ?\n");
		return;
	}

	ret = drm_hdmi_avi_infoframe_from_display_mode(&frame.avi,
						       mode, is_hdmi2_sink);
	if (ret < 0) {
		DRM_ERROR("couldn't fill AVI infoframe\n");
		return;
	}

	drm_hdmi_avi_infoframe_quant_range(&frame.avi, mode,
					   crtc_state->limited_color_range ?
					   HDMI_QUANTIZATION_RANGE_LIMITED :
					   HDMI_QUANTIZATION_RANGE_FULL,
					   false, is_hdmi2_sink);

	ret = hdmi_infoframe_pack(&frame, buf, sizeof(buf));
	if (ret < 0) {
		DRM_ERROR("Failed to pack AVI IF\n");
		return;
	}

	dig_port->write_infoframe(encoder, crtc_state, HDMI_INFOFRAME_TYPE_AVI,
				  buf, ret);
}

bool lspcon_infoframe_enabled(struct intel_encoder *encoder,
			      const struct intel_crtc_state *pipe_config)
{
	return enc_to_intel_lspcon(&encoder->base)->active;
}

void lspcon_resume(struct intel_lspcon *lspcon)
{
	enum drm_lspcon_mode expected_mode;

	if (lspcon_wake_native_aux_ch(lspcon)) {
		expected_mode = DRM_LSPCON_MODE_PCON;
		lspcon_resume_in_pcon_wa(lspcon);
	} else {
		expected_mode = DRM_LSPCON_MODE_LS;
	}

	if (lspcon_wait_mode(lspcon, expected_mode) == DRM_LSPCON_MODE_PCON)
		return;

	if (lspcon_change_mode(lspcon, DRM_LSPCON_MODE_PCON))
		DRM_ERROR("LSPCON resume failed\n");
	else
		DRM_DEBUG_KMS("LSPCON resume success\n");
}

void lspcon_wait_pcon_mode(struct intel_lspcon *lspcon)
{
	lspcon_wait_mode(lspcon, DRM_LSPCON_MODE_PCON);
}

bool lspcon_init(struct intel_digital_port *intel_dig_port)
{
	struct intel_dp *dp = &intel_dig_port->dp;
	struct intel_lspcon *lspcon = &intel_dig_port->lspcon;
	struct drm_device *dev = intel_dig_port->base.base.dev;
	struct drm_i915_private *dev_priv = to_i915(dev);

	if (!HAS_LSPCON(dev_priv)) {
		DRM_ERROR("LSPCON is not supported on this platform\n");
		return false;
	}

	lspcon->active = false;
	lspcon->mode = DRM_LSPCON_MODE_INVALID;

	if (!lspcon_probe(lspcon)) {
		DRM_ERROR("Failed to probe lspcon\n");
		return false;
	}

	if (!intel_dp_read_dpcd(dp)) {
		DRM_ERROR("LSPCON DPCD read failed\n");
		return false;
	}

	if (!lspcon_detect_vendor(lspcon)) {
		DRM_ERROR("LSPCON vendor detection failed\n");
		return false;
	}

	lspcon->active = true;
	DRM_DEBUG_KMS("Success: LSPCON init\n");
	return true;
}
