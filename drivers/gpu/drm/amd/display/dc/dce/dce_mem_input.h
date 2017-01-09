/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */
#ifndef __DCE_MEM_INPUT_H__
#define __DCE_MEM_INPUT_H__

#define MI_DCE_BASE_REG_LIST(id)\
	SRI(GRPH_ENABLE, DCP, id),\
	SRI(GRPH_CONTROL, DCP, id),\
	SRI(GRPH_X_START, DCP, id),\
	SRI(GRPH_Y_START, DCP, id),\
	SRI(GRPH_X_END, DCP, id),\
	SRI(GRPH_Y_END, DCP, id),\
	SRI(GRPH_PITCH, DCP, id),\
	SRI(HW_ROTATION, DCP, id),\
	SRI(GRPH_SWAP_CNTL, DCP, id),\
	SRI(PRESCALE_GRPH_CONTROL, DCP, id),\
	SRI(DPG_PIPE_ARBITRATION_CONTROL1, DMIF_PG, id),\
	SRI(DPG_WATERMARK_MASK_CONTROL, DMIF_PG, id),\
	SRI(DPG_PIPE_URGENCY_CONTROL, DMIF_PG, id),\
	SRI(DPG_PIPE_STUTTER_CONTROL, DMIF_PG, id),\
	SRI(DMIF_BUFFER_CONTROL, PIPE, id)

#define MI_DCE_PTE_REG_LIST(id)\
	SRI(DVMM_PTE_CONTROL, DCP, id),\
	SRI(DVMM_PTE_ARB_CONTROL, DCP, id)

#define MI_DCE8_REG_LIST(id)\
	MI_DCE_BASE_REG_LIST(id),\
	SRI(DPG_PIPE_NB_PSTATE_CHANGE_CONTROL, DMIF_PG, id)

#define MI_DCE11_2_REG_LIST(id)\
	MI_DCE8_REG_LIST(id),\
	SRI(GRPH_PIPE_OUTSTANDING_REQUEST_LIMIT, DCP, id)

#define MI_DCE11_REG_LIST(id)\
	MI_DCE11_2_REG_LIST(id),\
	MI_DCE_PTE_REG_LIST(id)

struct dce_mem_input_registers {
	/* DCP */
	uint32_t GRPH_ENABLE;
	uint32_t GRPH_CONTROL;
	uint32_t GRPH_X_START;
	uint32_t GRPH_Y_START;
	uint32_t GRPH_X_END;
	uint32_t GRPH_Y_END;
	uint32_t GRPH_PITCH;
	uint32_t HW_ROTATION;
	uint32_t GRPH_SWAP_CNTL;
	uint32_t PRESCALE_GRPH_CONTROL;
	uint32_t GRPH_PIPE_OUTSTANDING_REQUEST_LIMIT;
	uint32_t DVMM_PTE_CONTROL;
	uint32_t DVMM_PTE_ARB_CONTROL;
	/* DMIF_PG */
	uint32_t DPG_PIPE_ARBITRATION_CONTROL1;
	uint32_t DPG_WATERMARK_MASK_CONTROL;
	uint32_t DPG_PIPE_URGENCY_CONTROL;
	uint32_t DPG_PIPE_NB_PSTATE_CHANGE_CONTROL;
	uint32_t DPG_PIPE_LOW_POWER_CONTROL;
	uint32_t DPG_PIPE_STUTTER_CONTROL;
	uint32_t DPG_PIPE_STUTTER_CONTROL2;
	/* DCI */
	uint32_t DMIF_BUFFER_CONTROL;
	/* MC_HUB */
	uint32_t MC_HUB_RDREQ_DMIF_LIMIT;
};

/* Set_Filed_for_Block */
#define SFB(blk_name, reg_name, field_name, post_fix)\
	.field_name = blk_name ## reg_name ## __ ## field_name ## post_fix

#define MI_GFX8_TILE_MASK_SH_LIST(mask_sh, blk)\
	SFB(blk, GRPH_CONTROL, GRPH_BANK_HEIGHT, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_MACRO_TILE_ASPECT, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_TILE_SPLIT, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_MICRO_TILE_MODE, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_PIPE_CONFIG, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_ARRAY_MODE, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_COLOR_EXPANSION_MODE, mask_sh)

#define MI_DCP_MASK_SH_LIST(mask_sh, blk)\
	SFB(blk, GRPH_ENABLE, GRPH_ENABLE, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_DEPTH, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_FORMAT, mask_sh),\
	SFB(blk, GRPH_CONTROL, GRPH_NUM_BANKS, mask_sh),\
	SFB(blk, GRPH_X_START, GRPH_X_START, mask_sh),\
	SFB(blk, GRPH_Y_START, GRPH_Y_START, mask_sh),\
	SFB(blk, GRPH_X_END, GRPH_X_END, mask_sh),\
	SFB(blk, GRPH_Y_END, GRPH_Y_END, mask_sh),\
	SFB(blk, GRPH_PITCH, GRPH_PITCH, mask_sh),\
	SFB(blk, HW_ROTATION, GRPH_ROTATION_ANGLE, mask_sh),\
	SFB(blk, GRPH_SWAP_CNTL, GRPH_RED_CROSSBAR, mask_sh),\
	SFB(blk, GRPH_SWAP_CNTL, GRPH_BLUE_CROSSBAR, mask_sh),\
	SFB(blk, PRESCALE_GRPH_CONTROL, GRPH_PRESCALE_SELECT, mask_sh),\
	SFB(blk, PRESCALE_GRPH_CONTROL, GRPH_PRESCALE_R_SIGN, mask_sh),\
	SFB(blk, PRESCALE_GRPH_CONTROL, GRPH_PRESCALE_G_SIGN, mask_sh),\
	SFB(blk, PRESCALE_GRPH_CONTROL, GRPH_PRESCALE_B_SIGN, mask_sh)

#define MI_DCP_DCE11_MASK_SH_LIST(mask_sh, blk)\
	SFB(blk, GRPH_PIPE_OUTSTANDING_REQUEST_LIMIT, GRPH_PIPE_OUTSTANDING_REQUEST_LIMIT, mask_sh)

#define MI_DCP_PTE_MASK_SH_LIST(mask_sh, blk)\
	SFB(blk, DVMM_PTE_CONTROL, DVMM_PAGE_WIDTH, mask_sh),\
	SFB(blk, DVMM_PTE_CONTROL, DVMM_PAGE_HEIGHT, mask_sh),\
	SFB(blk, DVMM_PTE_CONTROL, DVMM_MIN_PTE_BEFORE_FLIP, mask_sh),\
	SFB(blk, DVMM_PTE_ARB_CONTROL, DVMM_PTE_REQ_PER_CHUNK, mask_sh),\
	SFB(blk, DVMM_PTE_ARB_CONTROL, DVMM_MAX_PTE_REQ_OUTSTANDING, mask_sh)

#define MI_DMIF_PG_MASK_SH_LIST(mask_sh, blk)\
	SFB(blk, DPG_PIPE_ARBITRATION_CONTROL1, PIXEL_DURATION, mask_sh),\
	SFB(blk, DPG_WATERMARK_MASK_CONTROL, URGENCY_WATERMARK_MASK, mask_sh),\
	SFB(blk, DPG_WATERMARK_MASK_CONTROL, STUTTER_EXIT_SELF_REFRESH_WATERMARK_MASK, mask_sh),\
	SFB(blk, DPG_PIPE_URGENCY_CONTROL, URGENCY_LOW_WATERMARK, mask_sh),\
	SFB(blk, DPG_PIPE_URGENCY_CONTROL, URGENCY_HIGH_WATERMARK, mask_sh),\
	SFB(blk, DPG_PIPE_STUTTER_CONTROL, STUTTER_ENABLE, mask_sh),\
	SFB(blk, DPG_PIPE_STUTTER_CONTROL, STUTTER_IGNORE_FBC, mask_sh),\
	SF(PIPE0_DMIF_BUFFER_CONTROL, DMIF_BUFFERS_ALLOCATED, mask_sh),\
	SF(PIPE0_DMIF_BUFFER_CONTROL, DMIF_BUFFERS_ALLOCATION_COMPLETED, mask_sh)

#define MI_DMIF_PG_MASK_SH_DCE(mask_sh, blk)\
	SFB(blk, DPG_PIPE_STUTTER_CONTROL, STUTTER_EXIT_SELF_REFRESH_WATERMARK, mask_sh),\
	SFB(blk, DPG_WATERMARK_MASK_CONTROL, NB_PSTATE_CHANGE_WATERMARK_MASK, mask_sh),\
	SFB(blk, DPG_PIPE_NB_PSTATE_CHANGE_CONTROL, NB_PSTATE_CHANGE_ENABLE, mask_sh),\
	SFB(blk, DPG_PIPE_NB_PSTATE_CHANGE_CONTROL, NB_PSTATE_CHANGE_URGENT_DURING_REQUEST, mask_sh),\
	SFB(blk, DPG_PIPE_NB_PSTATE_CHANGE_CONTROL, NB_PSTATE_CHANGE_NOT_SELF_REFRESH_DURING_REQUEST, mask_sh),\
	SFB(blk, DPG_PIPE_NB_PSTATE_CHANGE_CONTROL, NB_PSTATE_CHANGE_WATERMARK, mask_sh)

#define MI_DCE8_MASK_SH_LIST(mask_sh)\
	MI_DCP_MASK_SH_LIST(mask_sh, ),\
	MI_DMIF_PG_MASK_SH_LIST(mask_sh, ),\
	MI_DMIF_PG_MASK_SH_DCE(mask_sh, ),\
	MI_GFX8_TILE_MASK_SH_LIST(mask_sh, )

#define MI_DCE11_2_MASK_SH_LIST(mask_sh)\
	MI_DCE8_MASK_SH_LIST(mask_sh),\
	MI_DCP_DCE11_MASK_SH_LIST(mask_sh, )

#define MI_DCE11_MASK_SH_LIST(mask_sh)\
	MI_DCE11_2_MASK_SH_LIST(mask_sh),\
	MI_DCP_PTE_MASK_SH_LIST(mask_sh, )

#define MI_REG_FIELD_LIST(type) \
	type GRPH_ENABLE; \
	type GRPH_X_START; \
	type GRPH_Y_START; \
	type GRPH_X_END; \
	type GRPH_Y_END; \
	type GRPH_PITCH; \
	type GRPH_ROTATION_ANGLE; \
	type GRPH_RED_CROSSBAR; \
	type GRPH_BLUE_CROSSBAR; \
	type GRPH_PRESCALE_SELECT; \
	type GRPH_PRESCALE_R_SIGN; \
	type GRPH_PRESCALE_G_SIGN; \
	type GRPH_PRESCALE_B_SIGN; \
	type GRPH_PIPE_OUTSTANDING_REQUEST_LIMIT; \
	type DVMM_PAGE_WIDTH; \
	type DVMM_PAGE_HEIGHT; \
	type DVMM_MIN_PTE_BEFORE_FLIP; \
	type DVMM_PTE_REQ_PER_CHUNK; \
	type DVMM_MAX_PTE_REQ_OUTSTANDING; \
	type GRPH_DEPTH; \
	type GRPH_FORMAT; \
	type GRPH_NUM_BANKS; \
	type GRPH_BANK_WIDTH;\
	type GRPH_BANK_HEIGHT;\
	type GRPH_MACRO_TILE_ASPECT;\
	type GRPH_TILE_SPLIT;\
	type GRPH_MICRO_TILE_MODE;\
	type GRPH_PIPE_CONFIG;\
	type GRPH_ARRAY_MODE;\
	type GRPH_COLOR_EXPANSION_MODE;\
	type GRPH_SW_MODE; \
	type GRPH_SE_ENABLE; \
	type GRPH_NUM_SHADER_ENGINES; \
	type GRPH_NUM_PIPES; \
	type PIXEL_DURATION; \
	type URGENCY_WATERMARK_MASK; \
	type PSTATE_CHANGE_WATERMARK_MASK; \
	type NB_PSTATE_CHANGE_WATERMARK_MASK; \
	type STUTTER_EXIT_SELF_REFRESH_WATERMARK_MASK; \
	type URGENCY_LOW_WATERMARK; \
	type URGENCY_HIGH_WATERMARK; \
	type NB_PSTATE_CHANGE_ENABLE; \
	type NB_PSTATE_CHANGE_URGENT_DURING_REQUEST; \
	type NB_PSTATE_CHANGE_NOT_SELF_REFRESH_DURING_REQUEST; \
	type NB_PSTATE_CHANGE_WATERMARK; \
	type PSTATE_CHANGE_ENABLE; \
	type PSTATE_CHANGE_URGENT_DURING_REQUEST; \
	type PSTATE_CHANGE_NOT_SELF_REFRESH_DURING_REQUEST; \
	type PSTATE_CHANGE_WATERMARK; \
	type STUTTER_ENABLE; \
	type STUTTER_IGNORE_FBC; \
	type STUTTER_EXIT_SELF_REFRESH_WATERMARK; \
	type DMIF_BUFFERS_ALLOCATED; \
	type DMIF_BUFFERS_ALLOCATION_COMPLETED; \
	type ENABLE; /* MC_HUB_RDREQ_DMIF_LIMIT */\

struct dce_mem_input_shift {
	MI_REG_FIELD_LIST(uint8_t)
};

struct dce_mem_input_mask {
	MI_REG_FIELD_LIST(uint32_t)
};

struct dce_mem_input_wa {
	uint8_t single_head_rdreq_dmif_limit;
};

struct mem_input;

void dce_mem_input_program_pte_vm(struct mem_input *mi,
	enum surface_pixel_format format,
	union dc_tiling_info *tiling_info,
	enum dc_rotation_angle rotation);

void dce_mem_input_program_surface_config(struct mem_input *mi,
	enum surface_pixel_format format,
	union dc_tiling_info *tiling_info,
	union plane_size *plane_size,
	enum dc_rotation_angle rotation,
	struct dc_plane_dcc_param *dcc,
	bool horizontal_mirror,
	bool visible);

void dce_mem_input_allocate_dmif(struct mem_input *mi,
	uint32_t h_total,
	uint32_t v_total,
	uint32_t pix_clk_khz,
	uint32_t total_stream_num);

void dce_mem_input_free_dmif(struct mem_input *mi,
	uint32_t total_stream_num);

void dce_mem_input_program_display_marks(struct mem_input *mi,
	struct bw_watermarks nbp,
	struct bw_watermarks stutter,
	struct bw_watermarks urgent,
	uint32_t total_dest_line_time_ns);

#endif /*__DCE_MEM_INPUT_H__*/
