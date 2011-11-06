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


#ifndef _I18N_NATIVENUMBERSUPPLIER_HXX_
#define _I18N_NATIVENUMBERSUPPLIER_HXX_

#include <com/sun/star/i18n/XNativeNumberSupplier.hpp>
#include <com/sun/star/i18n/NativeNumberMode.hpp>
#include <com/sun/star/i18n/NativeNumberXmlAttributes.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

//      ----------------------------------------------------
//      class NativeNumberSupplier
//      ----------------------------------------------------
class NativeNumberSupplier : public cppu::WeakImplHelper2 
<
        com::sun::star::i18n::XNativeNumberSupplier,
        com::sun::star::lang::XServiceInfo
>
{
public:
        NativeNumberSupplier(sal_Bool _useOffset = sal_False) : useOffset(_useOffset) {}

        // Methods
        virtual ::rtl::OUString SAL_CALL getNativeNumberString( const ::rtl::OUString& aNumberString,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL isValidNatNum( const ::com::sun::star::lang::Locale& aLocale, 
                sal_Int16 nNativeNumberMode )
                throw (::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::i18n::NativeNumberXmlAttributes SAL_CALL convertToXmlAttributes(
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode ) 
                throw (::com::sun::star::uno::RuntimeException);

        virtual sal_Int16 SAL_CALL convertFromXmlAttributes( 
                const ::com::sun::star::i18n::NativeNumberXmlAttributes& aAttr )
                throw (::com::sun::star::uno::RuntimeException);

        //XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName() 
                throw( com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) 
                throw( com::sun::star::uno::RuntimeException );
        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() 
                throw( com::sun::star::uno::RuntimeException );

        // following methods are not for XNativeNumberSupplier, they are for calling from transliterations
        ::rtl::OUString SAL_CALL getNativeNumberString( const ::rtl::OUString& aNumberString,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode,
                com::sun::star::uno::Sequence< sal_Int32 >& offset  )
                throw (::com::sun::star::uno::RuntimeException);
        sal_Unicode SAL_CALL getNativeNumberChar( const sal_Unicode inChar,
                const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nNativeNumberMode )
                throw(com::sun::star::uno::RuntimeException) ;

private:
        ::com::sun::star::lang::Locale aLocale;
        sal_Bool useOffset;
};

} } } } 

#endif
