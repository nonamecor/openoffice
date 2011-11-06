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


#ifndef _I18N_TEXTCONERSION_HXX_
#define _I18N_TEXTCONERSION_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XExtendedTextConversion.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

namespace com { namespace sun { namespace star { namespace i18n {

//	----------------------------------------------------
//	class TextConversion
//	----------------------------------------------------
class TextConversionImpl : public cppu::WeakImplHelper2
<
	com::sun::star::i18n::XExtendedTextConversion,
	com::sun::star::lang::XServiceInfo
>
{
public:
	TextConversionImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ) : xMSF(rxMSF) {};

        // Methods
        com::sun::star::i18n::TextConversionResult SAL_CALL 
        getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, 
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType, 
            sal_Int32 nTextConversionOptions ) 
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL 
        getConversion( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, 
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType, 
            sal_Int32 nTextConversionOptions ) 
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        rtl::OUString SAL_CALL 
        getConversionWithOffset( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, 
            const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nTextConversionType, 
            sal_Int32 nTextConversionOptions, com::sun::star::uno::Sequence< sal_Int32 >& offset ) 
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );
        sal_Bool SAL_CALL 
        interactiveConversion( const ::com::sun::star::lang::Locale& aLocale,
            sal_Int16 nTextConversionType, sal_Int32 nTextConversionOptions ) 
            throw(  com::sun::star::uno::RuntimeException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::lang::NoSupportException );

	//XServiceInfo
	rtl::OUString SAL_CALL 
        getImplementationName() 
            throw( com::sun::star::uno::RuntimeException );
	sal_Bool SAL_CALL 
        supportsService(const rtl::OUString& ServiceName) 
            throw( com::sun::star::uno::RuntimeException );
	com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL 
        getSupportedServiceNames()
            throw( com::sun::star::uno::RuntimeException );
private :
	com::sun::star::lang::Locale aLocale;
	com::sun::star::uno::Reference < com::sun::star::i18n::XExtendedTextConversion > xTC;
	com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;

	void SAL_CALL getLocaleSpecificTextConversion( const com::sun::star::lang::Locale& rLocale )
            throw( com::sun::star::lang::NoSupportException );
};

} // i18n
} // star
} // sun
} // com


#endif
