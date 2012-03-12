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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <SerfPropFindReqProcImpl.hxx>
#include <SerfTypes.hxx>
#include <DAVProperties.hxx>

#include <webdavresponseparser.hxx>
#include <comphelper/seqstream.hxx>

using namespace com::sun::star;

namespace http_dav_ucp
{

SerfPropFindReqProcImpl::SerfPropFindReqProcImpl( const char* inPath,
                                                  const Depth inDepth,
                                                  const std::vector< ::rtl::OUString > & inPropNames,
                                                  std::vector< DAVResource > & ioResources )
    : SerfRequestProcessorImpl( inPath )
    , mDepthStr( 0 )
    , mpPropNames( &inPropNames )
    , mpResources( &ioResources )
    , mpResInfo( 0 )
    , mbOnlyPropertyNames( false )
    , xInputStream( new SerfInputStream() )
{
    init( inDepth );
}

SerfPropFindReqProcImpl::SerfPropFindReqProcImpl( const char* inPath,
                                                  const Depth inDepth,
                                                  std::vector< DAVResourceInfo > & ioResInfo )
    : SerfRequestProcessorImpl( inPath )
    , mDepthStr( 0 )
    , mpPropNames( 0 )
    , mpResources( 0 )
    , mpResInfo( &ioResInfo )
    , mbOnlyPropertyNames( true )
    , xInputStream( new SerfInputStream() )
{
    init( inDepth );
}

void SerfPropFindReqProcImpl::init( const Depth inDepth )
{
    switch ( inDepth )
    {
        case DAVZERO:
            mDepthStr = "0";
            break;
        case DAVONE:
            mDepthStr = "1";
            break;
        case DAVINFINITY:
            mDepthStr = "infinity";
            break;
    }
}

SerfPropFindReqProcImpl::~SerfPropFindReqProcImpl()
{
}

#define PROPFIND_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\"?><propfind xmlns=\"DAV:\">"
#define PROPFIND_TRAILER "</propfind>"

serf_bucket_t * SerfPropFindReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    // body bucket - certain properties OR all properties OR only property names
    serf_bucket_t* body_bkt = 0;
    rtl::OUString aBodyText;
    {
        // create and fill body bucket with requested properties
        const int nPropCount = ( !mbOnlyPropertyNames && mpPropNames )
                               ? mpPropNames->size() 
                               : 0;
        if ( nPropCount > 0 )
        {
            SerfPropName thePropName;
            for ( int theIndex = 0; theIndex < nPropCount; theIndex ++ )
            {
                // split fullname into namespace and name!
                DAVProperties::createSerfPropName( (*mpPropNames)[ theIndex ], 
                                                   thePropName );

                /* <*propname* xmlns="*propns*" /> */
                aBodyText += rtl::OUString::createFromAscii( "<" );
                aBodyText += rtl::OUString::createFromAscii( thePropName.name );
                aBodyText += rtl::OUString::createFromAscii( " xmlnx=\"" );
                aBodyText += rtl::OUString::createFromAscii( thePropName.nspace );
                aBodyText += rtl::OUString::createFromAscii( "\"/>" );
            }

            aBodyText = rtl::OUString::createFromAscii( "<prop>" ) + 
                        aBodyText +
                        rtl::OUString::createFromAscii( "</prop>" );
        }
        else
        {
            if ( mbOnlyPropertyNames )
            {
                aBodyText = rtl::OUString::createFromAscii( "<propname/>" );
            }
            else
            {
                aBodyText = rtl::OUString::createFromAscii( "<allprop/>" );
            }
        }

        aBodyText = rtl::OUString::createFromAscii( PROPFIND_HEADER ) + 
                    aBodyText +
                    rtl::OUString::createFromAscii( PROPFIND_TRAILER );
        body_bkt = SERF_BUCKET_SIMPLE_STRING( rtl::OUStringToOString( aBodyText, RTL_TEXTENCODING_UTF8 ), 
                                              pSerfBucketAlloc );
        if ( useChunkedEncoding() )
        {
            body_bkt = serf_bucket_chunk_create( body_bkt, pSerfBucketAlloc );
        }
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest, 
                                                                 "PROPFIND",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 pSerfBucketAlloc );

    // TODO - correct header data
    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    serf_bucket_headers_setn( hdrs_bkt, "User-Agent", "www.openoffice.org/ucb/" );
    serf_bucket_headers_setn( hdrs_bkt, "Accept-Encoding", "gzip");

    // request specific header fields
    // request specific header fields
    serf_bucket_headers_setn( hdrs_bkt, "Depth", mDepthStr );
    if ( body_bkt != 0 && aBodyText.getLength() > 0 )
    {
        if ( useChunkedEncoding() )
        {
            serf_bucket_headers_setn( hdrs_bkt, "Transfer-Encoding", "chunked");
        }
        serf_bucket_headers_setn( hdrs_bkt, "Content-Type", "application/xml" );
        serf_bucket_headers_setn( hdrs_bkt, "Content-Length", 
                                  rtl::OUStringToOString( rtl::OUString::valueOf( aBodyText.getLength() ), RTL_TEXTENCODING_UTF8 ) );
    }

    return req_bkt;
}

void SerfPropFindReqProcImpl::processChunkOfResponseData( const char* data, 
                                                          apr_size_t len )
{
    if ( xInputStream.is() )
    {
        xInputStream->AddToStream( data, len );
    }
}

void SerfPropFindReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    if ( mbOnlyPropertyNames )
    {
        const std::vector< DAVResourceInfo > rResInfo( parseWebDAVPropNameResponse( xInputStream.get() ) );
        *mpResInfo = rResInfo;
    }
    else
    {
        const std::vector< DAVResource > rResources( parseWebDAVPropFindResponse( xInputStream.get() ) );
        *mpResources = rResources;
    }
}

} // namespace http_dav_ucp
