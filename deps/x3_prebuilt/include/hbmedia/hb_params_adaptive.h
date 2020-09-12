#ifndef _HB_VENC_PARAMS_ADAPTIVE_H__
#define _HB_VENC_PARAMS_ADAPTIVE_H__

#include "hb_comm_vdec.h"
#include "hb_comm_venc.h"
#include "hb_media_codec.h"

int32_t hb_venc_attr_adaptive_context(
    const VENC_ATTR_S *pstVencAttr,
    mc_video_codec_enc_params_t *pst_video_enc_params);

int32_t hb_context_adaptive_venc_attr(const media_codec_context_t *ctx,
                                      VENC_ATTR_S *pstVencAttr);

int32_t hb_rc_params_adaptive_context(const VENC_RC_ATTR_S *pstRcAttr,
                                      mc_rate_control_params_t *pst_rc_params);

int32_t hb_context_adaptive_rc_params(
    const mc_rate_control_params_t *pst_rc_params, VENC_RC_ATTR_S *pstRcAttr);

int32_t hb_gop_params_adaptive_context(const VENC_GOP_ATTR_S *pstRcAttr,
                                       mc_video_gop_params_t *pst_gop_params);

int32_t hb_context_adaptive_gop_params(
    const mc_video_gop_params_t *pst_gop_params, VENC_GOP_ATTR_S *pstRcAttr);

int32_t hb_venc_chn_attr_adaptive_context(const VENC_CHN_ATTR_S *pstVencAttr,
                                          media_codec_context_t *ctx);

int32_t hb_contex_adaptivet_venc_chn_attr(const media_codec_context_t *ctx,
                                          VENC_CHN_ATTR_S *pstVencAttr);

int32_t hb_vdec_chn_attr_adaptive_context(const VDEC_CHN_ATTR_S *pstVdecAttr,
                                          media_codec_context_t *ctx);

int hb_context_adaptive_vdec_chn_attr(const media_codec_context_t *ctx,
                                      VDEC_CHN_ATTR_S *pstVdecAttr);

#endif  // _HB_VENC_PARAMS_ADAPTIVE_H__
