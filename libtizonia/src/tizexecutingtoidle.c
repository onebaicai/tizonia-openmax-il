/**
 * Copyright (C) 2011-2013 Aratelia Limited - Juan A. Rubio
 *
 * This file is part of Tizonia
 *
 * Tizonia is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Tizonia is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tizonia.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   tizexecutingtoidle.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia OpenMAX IL - ExecutingToIdle OMX IL substate implementation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "OMX_TizoniaExt.h"

#include "tizexecutingtoidle.h"
#include "tizstate_decls.h"
#include "tizkernel.h"
#include "tizosal.h"

#include <assert.h>

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.tizonia.fsm.executingtoidle"
#endif


static void *
executingtoidle_ctor (void *ap_obj, va_list * app)
{
  tiz_executingtoidle_t *p_obj =
    super_ctor (tizexecutingtoidle, ap_obj, app);
  return p_obj;
}

static void *
executingtoidle_dtor (void *ap_obj)
{
  return super_dtor (tizexecutingtoidle, ap_obj);
}

static OMX_ERRORTYPE
executingtoidle_GetState (const void *ap_obj,
                          OMX_HANDLETYPE ap_hdl, OMX_STATETYPE * ap_state)
{
  assert (NULL != ap_state);
  *ap_state = OMX_StateExecuting;
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
executingtoidle_UseBuffer (const void *ap_obj,
                           OMX_HANDLETYPE ap_hdl,
                           OMX_BUFFERHEADERTYPE ** app_buf_hdr,
                           OMX_U32 a_port_index,
                           OMX_PTR ap_app_private,
                           OMX_U32 a_size_bytes, OMX_U8 * ap_buf)
{
  return OMX_ErrorNotImplemented;
}

/*
 * initialization
 */

/*
 * from tiz_state
 */

static OMX_ERRORTYPE
executingtoidle_trans_complete (const void *ap_obj,
                                    OMX_PTR ap_servant,
                                    OMX_STATETYPE a_new_state)
{
  const tiz_state_t *p_base = (const tiz_state_t *) ap_obj;

  TIZ_LOG_CNAME (TIZ_DEBUG, TIZ_CNAME (tiz_srv_get_hdl(ap_servant)),
                 TIZ_CBUF (tiz_srv_get_hdl(ap_servant)),
                 "Trans complete to state [%s]...",
                 tiz_fsm_state_to_str (a_new_state));

  assert (NULL != ap_obj);
  assert (NULL != ap_servant);
  assert (OMX_StateIdle == a_new_state);

  if (2 == p_base->servants_count_ + 1)
    {
      /* Reset the OMX_TIZONIA_PORTSTATUS_AWAITBUFFERSRETURN flag in all ports
         where this has been set */
      tiz_krn_reset_tunneled_ports_status
        (tiz_get_krn (tiz_srv_get_hdl(ap_servant)),
         OMX_TIZONIA_PORTSTATUS_AWAITBUFFERSRETURN);
    }

  return tiz_state_super_trans_complete (tizexecutingtoidle, ap_obj,
                                        ap_servant, a_new_state);
}

static OMX_ERRORTYPE
executingtoidle_tunneled_ports_status_update (void *ap_obj)
{
  tiz_state_t *p_base = (tiz_state_t *) ap_obj;

  assert (NULL != ap_obj);

  {
    OMX_HANDLETYPE p_hdl = tiz_srv_get_hdl(p_base->p_fsm_);
    void *p_krn = tiz_get_krn (p_hdl);

    if (TIZ_KRN_MAY_INIT_EXE_TO_IDLE (p_krn))
      {
        /* OK, at this point all the tunneled non-supplier neighboring ports
           are ready to receive ETB/FTB calls.  NOTE: This will call the
         * 'tiz_state_state_set' function of the tiz_state_t base class (note
         * we are passing 'tizidle' as 1st parameter */
        TIZ_LOG_CNAME (TIZ_TRACE, TIZ_CNAME (p_hdl), TIZ_CBUF (p_hdl),
                       "kernel may initiate exe to idle");
        return tiz_state_super_state_set (tizidle, ap_obj, p_hdl,
                                         OMX_CommandStateSet,
                                         OMX_StateIdle, NULL);
      }
  }

  return OMX_ErrorNone;
}


/*
 * initialization
 */

const void *tizexecutingtoidle;

OMX_ERRORTYPE
tiz_executingtoidle_init (void)
{
  if (!tizexecutingtoidle)
    {
      tiz_check_omx_err_ret_oom (tiz_executing_init ());
      tiz_check_null_ret_oom
        (tizexecutingtoidle =
         factory_new
         (tizstate_class, "tizexecutingtoidle",
          tizexecuting, sizeof (tiz_executingtoidle_t),
          ctor, executingtoidle_ctor,
          dtor, executingtoidle_dtor,
          tiz_api_GetState, executingtoidle_GetState,
          tiz_api_UseBuffer, executingtoidle_UseBuffer,
          tiz_state_trans_complete, executingtoidle_trans_complete,
          tiz_state_tunneled_ports_status_update, executingtoidle_tunneled_ports_status_update,
          0));
    }
  return OMX_ErrorNone;
}
