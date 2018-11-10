/**
 * This file is part of the "signaltk" project
 *   Copyright (c) 2018 Paul Asmuth
 *
 * libstx is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#pragma once

namespace signaltk {

template <typename T>
Status Context::element_get(T** elem) {
  if (elements.empty()) {
    return Status::ERROR_INVALID_ELEM;
  }

  *elem = static_cast<T*>(elements.top().get());
  return OK;
}

template <typename T>
Status Context::element_get(T const** elem) const {
  if (elements.empty()) {
    return Status::ERROR_INVALID_ELEM;
  }

  *elem = static_cast<const T*>(elements.top().get());
  return OK;
}

} // namespace signaltk
