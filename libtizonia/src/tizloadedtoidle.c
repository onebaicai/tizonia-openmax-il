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
 * @file   tizloadedtoidle.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia OpenMAX IL - LoadedToIdle OMX IL substate implementation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include "tizloadedtoidle.h"
#include "tizstate_decls.h"
#include "tizutils.h"
#include "tizkernel.h"

#include "tizosal.h"

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.tizonia.fsm.loadedtoidle"
#endif


static void *
loadedtoidle_ctor (void *ap_obj, va_list * app)
{
  tiz_loadedtoidle_t *p_obj = super_ctor (tizloadedtoidle, ap_obj, app);
  return p_obj;
}

static void *
loadedtoidle_dtor (void *ap_obj)
{
  return super_dtor (tizloadedtoidle, ap_obj);
}

static OMX_ERRORTYPE
loadedtoidle_SetParameter (const void *ap_obj,
                           OMX_HANDLETYPE ap_hdl,
                           OMX_INDEXTYPE a_index, OMX_PTR ap_struct)
{
  /* In this transitional state, OMX_SetParameter should only be allowed */
  /* until the first OMX_UseBuffer call is received */
  TIZ_LOG_CNAME (TIZ_TRACE, TIZ_CNAME (ap_hdl), TIZ_CBUF (ap_hdl),
                 "[%s]", tiz_idx_to_str (a_index));

  return super_SetParameter (tizloadedtoidle, ap_obj, ap_hdl, a_index,
                             ap_struct);
}

static OMX_ERRORTYPE
loadedtoidle_GetState (const void *ap_obj,
                       OMX_HANDLETYPE ap_hdl, OMX_STATETYPE * ap_state)
{
  assert (NULL != ap_state);
  *ap_state = OMX_StateLoaded;
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
loadedtoidle_UseBuffer (const void *ap_obj,
                        OMX_HANDLETYPE ap_hdl,
                        OMX_BUFFERHEADERTYPE ** app_buf_hdr,
                        OMX_U32 a_port_index,
                        OMX_PTR ap_app_private,
                        OMX_U32 a_size_bytes, OMX_U8 * ap_buf)
{
  return OMX_ErrorNotImplemented;
}

static OMX_ERRORTYPE
loadedtoidle_EmptyThisBuffer (const void *ap_obj,
                              OMX_HANDLETYPE ap_hdl,
                              OMX_BUFFERHEADERTYPE * ap_buf)
{
  return OMX_ErrorNotImplemented;
}

static OMX_ERRORTYPE
loadedtoidle_FillThisBuffer (const void *ap_obj,
                             OMX_HANDLETYPE ap_hdl,
                             OMX_BUFFERHEADERTYPE * ap_buf)
{
  return OMX_ErrorNotImplemented;
}

/*
 * from tiz_state class
 */

static OMX_ERRORTYPE
loadedtoidle_state_set (const void *ap_obj,
                        OMX_HANDLETYPE ap_hdl,
                        OMX_COMMANDTYPE a_cmd,
                        OMX_U32 a_param1, OMX_PTR ap_cmd_data)
{
  tiz_state_t *p_base = (tiz_state_t *) ap_obj;
  tiz_fsm_state_id_t new_state = EStateMax;

  assert (NULL != ap_obj);
  assert (NULL != ap_hdl);
  assert (a_cmd == OMX_CommandStateSet);

  TIZ_LOG_CNAME (TIZ_TRACE, TIZ_CNAME (ap_hdl), TIZ_CBUF (ap_hdl),
                 "Requested transition [ESubStateLoadedToIdle -> %s]...",
                 tiz_fsm_state_to_str (a_param1));

  /* Allowed transitions are OMX_StateLoaded only (a.k.a. transition
   * cancellation). */
  switch (a_param1)
    {
    case OMX_StateLoaded:
      {
        new_state = ESubStateIdleToLoaded;
      }
      break;

    default:
      {
        TIZ_LOG_CNAME (TIZ_ERROR, TIZ_CNAME (ap_hdl), TIZ_CBUF (ap_hdl),
                       "[OMX_ErrorIncorrectStateTransition] : "
                       "ESubStateLoadedToIdle -> [%s]",
                       tiz_state_to_str (a_param1));
        return OMX_ErrorIncorrectStateTransition;
      }

    };

  /* reset here the servants count */
  p_base->servants_count_ = 0;

  if (ESubStateIdleToLoaded == new_state)
    {
      OMX_ERRORTYPE rc = OMX_ErrorNone;
      if (OMX_ErrorNone !=
          (rc = tiz_fsm_set_state
           (tiz_get_fsm (ap_hdl), new_state, ESubStateLoadedToIdle)))
        {
          return rc;
        }
    }

  /* IL resource deallocation should take place now */
  /* NOTE: This will call the 'tiz_state_state_set' function and not
   * 'tizloaded_state_set' (we are passing 'tizloaded' as the 1st
   * parameter  */
  return tiz_state_super_state_set (tizloaded, ap_obj, ap_hdl, a_cmd,
                                   a_param1, ap_cmd_data);
}

static OMX_ERRORTYPE
loadedtoidle_trans_complete (const void *ap_obj,
                             OMX_PTR ap_servant, OMX_STATETYPE a_new_state)
{
  const tiz_state_t *p_base = (const tiz_state_t *) ap_obj;

  TIZ_LOG_CNAME (TIZ_TRACE, TIZ_CNAME (tiz_srv_get_hdl (ap_servant)),
                 TIZ_CBUF (tiz_srv_get_hdl (ap_servant)),
                 "Trans complete to state [%s]...",
                 tiz_fsm_state_to_str (a_new_state));

  assert (NULL != ap_obj);
  assert (NULL != ap_servant);
  assert (OMX_StateIdle == a_new_state);

  if (2 == p_base->servants_count_ + 1)
    {
      /* Reset the OMX_PORTSTATUS_ACCEPTUSEBUFFER flag in all ports where this
       * has been set */
      tiz_krn_reset_tunneled_ports_status (tiz_get_krn
                                             (tiz_srv_get_hdl (ap_servant)),
                                             OMX_PORTSTATUS_ACCEPTUSEBUFFER);
    }

  return tiz_state_super_trans_complete (tizloadedtoidle, ap_obj, ap_servant,
                                        a_new_state);
}

static OMX_ERRORTYPE
loadedtoidle_tunneled_ports_status_update (void *ap_obj)
{
  tiz_state_t *p_base = (tiz_state_t *) ap_obj;

  assert (NULL != ap_obj);

  {
    OMX_HANDLETYPE p_hdl = tiz_srv_get_hdl (p_base->p_fsm_);
    void *p_krn = tiz_get_krn (p_hdl);

    if (TIZ_KRN_MAY_INIT_ALLOC_PHASE (p_krn))
      {
        /* OK, at this point all the tunneled non-supplier neighboring ports
         * are ready to receive OMX_UseBuffer calls. IL resource allocation
         * will take place now */
        /* NOTE: This will call the 'tiz_state_state_set' function of the base
         * class (we are passing 'tizloaded' as the 1st parameter */
        return tiz_state_super_state_set (tizloaded, ap_obj, p_hdl,
                                         OMX_CommandStateSet,
                                         OMX_StateIdle, NULL);
      }
  }

  return OMX_ErrorNone;
}


/*
 * initialization
 */

const void *tizloadedtoidle;

OMX_ERRORTYPE
tiz_loadedtoidle_init (void)
{
  if (!tizloadedtoidle)
    {
      tiz_check_omx_err_ret_oom (tiz_loaded_init ());
      tiz_check_null_ret_oom
        (tizloadedtoidle =
         factory_new
         (tizstate_class, "tizloadedtoidle",
          tizloaded, sizeof (tiz_loadedtoidle_t),
          ctor, loadedtoidle_ctor,
          dtor, loadedtoidle_dtor,
          tiz_api_SetParameter, loadedtoidle_SetParameter,
          tiz_api_GetState, loadedtoidle_GetState,
          tiz_api_UseBuffer, loadedtoidle_UseBuffer,
          tiz_api_EmptyThisBuffer, loadedtoidle_EmptyThisBuffer,
          tiz_api_FillThisBuffer, loadedtoidle_FillThisBuffer,
          tiz_state_state_set, loadedtoidle_state_set,
          tiz_state_trans_complete, loadedtoidle_trans_complete,
          tiz_state_tunneled_ports_status_update,
          loadedtoidle_tunneled_ports_status_update, 0));
    }
  return OMX_ErrorNone;
}
