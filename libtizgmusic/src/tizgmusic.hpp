/**
 * Copyright (C) 2011-2015 Aratelia Limited - Juan A. Rubio
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
 * @file   tizgmusic.hpp
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia - Simple Google Music client library
 *
 *
 */

#ifndef TIZGMUSIC_HPP
#define TIZGMUSIC_HPP

#include <boost/python.hpp>

#include <string>

class tizgmusic
{

public:
  tizgmusic (const std::string &user, const std::string &pass,
             const std::string &device_id);
  ~tizgmusic ();

  int init ();
  int start ();
  void stop ();
  void deinit ();

  int enqueue_album (const std::string &album);
  int enqueue_artist (const std::string &artist);
  void clear_queue ();

  const char * get_next_url ();
  const char * get_current_song_artist ();
  const char * get_current_song_title ();

private:
  int get_current_song ();

private:
  std::string user_;
  std::string pass_;
  std::string device_id_;
  std::string current_url_;
  std::string current_artist_;
  std::string current_title_;
  boost::python::object py_main_;
  boost::python::object py_global_;
  boost::python::object py_gm_proxy_;
};

#endif  // TIZGMUSIC_HPP