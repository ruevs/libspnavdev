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

#	set(CMAKE_EXECUTABLE_SUFFIX ".html")

    set(SHELL ${CMAKE_CURRENT_SOURCE_DIR}/test.html)

    set_target_properties(test PROPERTIES LINK_FLAGS
		"--bind
		--no-heap-copy -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -s ASYNCIFY=1
		-s DYNCALLS=1 -s ASSERTIONS=1
		-s EXPORTED_RUNTIME_METHODS=ccall,cwrap
		-s EXPORTED_FUNCTIONS=_test,_main
		-s TOTAL_STACK=33554432 -s TOTAL_MEMORY=134217728")
endif()
