# libspnavdev - direct 6dof device handling library
# Copyright (C) 2021 Collabora, Ltd.
#
# SPDX-License-Identifier: GPL-3-or-later
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

add_executable(test test.c)
target_link_libraries(test PRIVATE spnavdev)

if(EMSCRIPTEN)
    set(M_LIBRARY "" CACHE STRING "libm (not necessary)" FORCE)

    target_link_options(test PRIVATE
        "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/test.html"
        "--bind"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sWASM=1"
        "-sASYNCIFY=1"
        "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap"
        "-sEXPORTED_FUNCTIONS=_test,_main"
    )

    set_target_properties(test PROPERTIES
        SUFFIX ".html"
    )

    set_property(TARGET test APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/test.html"
    )
endif()

