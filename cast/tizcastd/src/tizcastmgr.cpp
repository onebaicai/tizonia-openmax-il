/**
 * Copyright (C) 2011-2017 Aratelia Limited - Juan A. Rubio
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
 * @file   tizcastmgr.cpp
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Cast manager impl
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include <algorithm>
#include <boost/make_shared.hpp>

#include <OMX_Component.h>

#include <tizmacros.h>
#include <tizplatform.h>

#include "tizcastmgr.hpp"
#include "tizcastmgrcmd.hpp"
#include "tizcastmgrops.hpp"

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.cast.mgr"
#endif

namespace cast = tiz::cast;

//
// mgr
//
cast::mgr::mgr (const tiz_chromecast_ctx_t *p_cc_ctx,
                const std::string &name_or_ip, cast_status_cback_t cast_cb,
                media_status_cback_t media_cb,
                termination_callback_t termination_cb)
  : p_ops_ (),
    fsm_ (boost::msm::back::states_, &p_ops_),
    name_or_ip_ (name_or_ip),
    cast_cb_ (cast_cb),
    media_cb_ (media_cb),
    termination_cb_ (termination_cb)
{
  TIZ_LOG (TIZ_PRIORITY_TRACE, "Constructing...");
}

cast::mgr::~mgr ()
{
  delete p_ops_;
  p_ops_ = NULL;
}

OMX_ERRORTYPE
cast::mgr::init ()
{
  // Init this mgr's operations using the do_init template method
  tiz_check_null_ret_oom (
      (p_ops_
       = new ops (this, p_cc_ctx_,
                  boost::bind (&tiz::cast::mgr::cast_status_received, this),
                  cast_cb_, media_cb_, termination_cb_)));

  // Get the fsm to start processing events
  return start_fsm ();
}

void cast::mgr::deinit ()
{
  if (!fsm_.terminated_)
    {
      (void)stop_fsm ();
    }
}

//
// Private methods
//

OMX_ERRORTYPE
cast::mgr::start_fsm ()
{
  return post_cmd (new cast::cmd (cast::start_evt ()));
}

OMX_ERRORTYPE
cast::mgr::stop_fsm ()
{
  return post_cmd (new cast::cmd (cast::quit_evt ()));
}

OMX_ERRORTYPE
cast::mgr::cast_status_received ()
{
  return post_cmd (new cast::cmd (cast::cast_status_evt ()));
}

bool cast::mgr::dispatch_cmd (const cast::cmd *p_cmd)
{
  assert (p_ops_);
  assert (p_cmd);

  p_cmd->inject (fsm_);

  // Check for internal errors produced during the processing of the last
  // event. If any, inject an "internal" error event. This is fatal and shall
  // terminate the state machine.
  if (OMX_ErrorNone != p_ops_->internal_error ())
  {
    TIZ_LOG (TIZ_PRIORITY_ERROR,
             "MGR error detected. Injecting err_evt (this is fatal)");
    bool is_internal_error = true;
    fsm_.process_event (cast::err_evt (
        p_ops_->internal_error (), p_ops_->internal_error_msg (),
        is_internal_error));
  }
  if (fsm_.terminated_)
  {
    TIZ_LOG (TIZ_PRIORITY_NOTICE, "MGR fsm terminated");
    p_ops_->deinit ();
  }

  return fsm_.terminated_;
}
