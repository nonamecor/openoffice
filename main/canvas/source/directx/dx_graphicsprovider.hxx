/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _DXCANVAS_GRAPHICSPROVIDER_HXX
#define _DXCANVAS_GRAPHICSPROVIDER_HXX

#include "dx_winstuff.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace Gdiplus{ class Graphics; }

namespace dxcanvas
{
    /** Provider of a Gdiplus::Graphics. Interface
     */
	class GraphicsProvider : private ::boost::noncopyable
    {
    public:
        virtual ~GraphicsProvider() {}

        virtual GraphicsSharedPtr getGraphics() = 0;
    };

    typedef ::boost::shared_ptr< GraphicsProvider > GraphicsProviderSharedPtr;
}

#endif /* _DXCANVAS_GRAPHICSPROVIDER_HXX */
