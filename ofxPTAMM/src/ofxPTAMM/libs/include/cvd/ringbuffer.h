/*                       
	This file is part of the CVD Library.

	Copyright (C) 2005 The Authors

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#include <vector>

namespace CVD
{


/// Implements a ringbuffer based on std::vector. A ringbuffer is a circular
/// buffer i.e. once the end of the buffer is reached, reading or writing continues
/// from the start of the buffer. The buffer maintains a notion of a 'current' element,
/// which is buffer[0], and one can access elements before this or after this (using [-n] or
/// [n] respectively), as well as advancing the current element slot by calling advance()
/// @param T The data type held by the buffer
/// @ingroup gCPP
template <typename T>
class RingBuffer {
 public:
  /// Constructor.
  /// @param size The size of the ringbuffer
  RingBuffer(int size=0){
    my_buffer.resize(size);
    my_start=0;
  }

  /// Constructor.
  /// @param size The size of the ringbuffer
  /// @param t The default value for all elements in the ringbuffer
  RingBuffer(int size, const T &t){
    my_buffer.resize(size,t);
    my_start=0;
  }

  ~RingBuffer(){}


  /// Resize the buffer
  /// @param size The size of the ringbuffer
  void resize(int size){
    my_buffer.resize(size);
  }

  /// What is the size of the buffer?
  int size() const {
    return my_buffer.size();
  }

  /// Access an element from the ringbuffer. The [0] element is the
  /// current element, and one can use [-n] to access previous values
  /// in the ringbuffer (up to a limit of -size(), naturally).
  /// @param i The element number
  T& operator[](int i){
    return my_buffer[(my_buffer.size()+i+my_start)%my_buffer.size()];
  }

  /// Access an element from the ringbuffer. The [0] element is the
  /// current element, and one can use [-n] to access previous values
  /// in the ringbuffer (up to a limit of -size(), naturally).
  /// @param i The element number
  const T& operator[](int i) const {
    return my_buffer[(my_buffer.size()+i+my_start)%my_buffer.size()];
  }

  /// Step the current element on by one.
  /// @param n The number of slots to advance by (default is 1)
  void advance(int n=1){
    my_start = (my_start+my_buffer.size()+n)%my_buffer.size();
  }
  
 private:
  std::vector<T> my_buffer;
  int my_start;
};


}

#endif

