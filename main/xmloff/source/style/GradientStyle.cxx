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
#include "precompiled_xmloff.hxx"
#include "xmloff/GradientStyle.hxx"
#include <com/sun/star/awt/Gradient.hpp>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>



using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
	XML_TOK_GRADIENT_NAME,
	XML_TOK_GRADIENT_DISPLAY_NAME,
	XML_TOK_GRADIENT_STYLE,
	XML_TOK_GRADIENT_CX,
	XML_TOK_GRADIENT_CY,
	XML_TOK_GRADIENT_STARTCOLOR,
	XML_TOK_GRADIENT_ENDCOLOR,
	XML_TOK_GRADIENT_STARTINT,
	XML_TOK_GRADIENT_ENDINT,
	XML_TOK_GRADIENT_ANGLE,
	XML_TOK_GRADIENT_BORDER,
	XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};


SvXMLEnumMapEntry __READONLY_DATA pXML_GradientStyle_Enum[] =
{
	{ XML_GRADIENTSTYLE_LINEAR,		    awt::GradientStyle_LINEAR },
	{ XML_GRADIENTSTYLE_AXIAL,			awt::GradientStyle_AXIAL },
	{ XML_GRADIENTSTYLE_RADIAL,		    awt::GradientStyle_RADIAL },
	{ XML_GRADIENTSTYLE_ELLIPSOID,		awt::GradientStyle_ELLIPTICAL },
	{ XML_GRADIENTSTYLE_SQUARE,		    awt::GradientStyle_SQUARE },
	{ XML_GRADIENTSTYLE_RECTANGULAR,	awt::GradientStyle_RECT },
	{ XML_TOKEN_INVALID, 0 }
};

//-------------------------------------------------------------
// Import
//-------------------------------------------------------------
XMLGradientStyleImport::XMLGradientStyleImport( 
    SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLGradientStyleImport::~XMLGradientStyleImport()
{
}

sal_Bool XMLGradientStyleImport::importXML( 
    const uno::Reference< xml::sax::XAttributeList >& xAttrList, 
    uno::Any& rValue, 
    OUString& rStrName )
{
	sal_Bool bRet           = sal_False;
	sal_Bool bHasName       = sal_False;
	sal_Bool bHasStyle      = sal_False;
	sal_Bool bHasStartColor = sal_False;
	sal_Bool bHasEndColor   = sal_False;
	OUString aDisplayName;
	
	awt::Gradient aGradient;
	aGradient.XOffset = 0;
	aGradient.YOffset = 0;
	aGradient.StartIntensity = 100;
	aGradient.EndIntensity = 100;
	aGradient.Angle = 0;
	aGradient.Border = 0;

    {
        static __FAR_DATA SvXMLTokenMapEntry aGradientAttrTokenMap[] =
{
	{ XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_GRADIENT_NAME },
	{ XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_GRADIENT_DISPLAY_NAME },
	{ XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_GRADIENT_STYLE },
	{ XML_NAMESPACE_DRAW, XML_CX, XML_TOK_GRADIENT_CX },
	{ XML_NAMESPACE_DRAW, XML_CY, XML_TOK_GRADIENT_CY },
	{ XML_NAMESPACE_DRAW, XML_START_COLOR, XML_TOK_GRADIENT_STARTCOLOR },
	{ XML_NAMESPACE_DRAW, XML_END_COLOR, XML_TOK_GRADIENT_ENDCOLOR },
	{ XML_NAMESPACE_DRAW, XML_START_INTENSITY, XML_TOK_GRADIENT_STARTINT },
	{ XML_NAMESPACE_DRAW, XML_END_INTENSITY, XML_TOK_GRADIENT_ENDINT },
	{ XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, XML_TOK_GRADIENT_ANGLE },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, XML_TOK_GRADIENT_BORDER },
	XML_TOKEN_MAP_END 
};

	SvXMLTokenMap aTokenMap( aGradientAttrTokenMap );
    SvXMLNamespaceMap& rNamespaceMap = rImport.GetNamespaceMap();

	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
		OUString aStrAttrName;
		sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
		const OUString& rStrValue = xAttrList->getValueByIndex( i );

		sal_Int32 nTmpValue;

		switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
		{
		case XML_TOK_GRADIENT_NAME:
			{
				rStrName = rStrValue;
				bHasName = sal_True;
			}			
			break;
		case XML_TOK_GRADIENT_DISPLAY_NAME:
			{
				aDisplayName = rStrValue;
			}			
			break;
		case XML_TOK_GRADIENT_STYLE:
			{
				sal_uInt16 eValue;
				if( SvXMLUnitConverter::convertEnum( eValue, rStrValue, pXML_GradientStyle_Enum ) )
				{
					aGradient.Style = (awt::GradientStyle) eValue;
					bHasStyle = sal_True;
				}
			}
			break;
		case XML_TOK_GRADIENT_CX:
			SvXMLUnitConverter::convertPercent( nTmpValue, rStrValue );
			aGradient.XOffset = static_cast< sal_Int16 >( nTmpValue );
			break;
		case XML_TOK_GRADIENT_CY:
			SvXMLUnitConverter::convertPercent( nTmpValue, rStrValue );
			aGradient.YOffset = static_cast< sal_Int16 >( nTmpValue );
			break;
		case XML_TOK_GRADIENT_STARTCOLOR:
			{
				Color aColor;
                bHasStartColor = SvXMLUnitConverter::convertColor( aColor, rStrValue );
				if( bHasStartColor )
					aGradient.StartColor = (sal_Int32)( aColor.GetColor() );
			}
			break;
		case XML_TOK_GRADIENT_ENDCOLOR:
			{
				Color aColor;
                bHasStartColor = SvXMLUnitConverter::convertColor( aColor, rStrValue );
				if( bHasStartColor )
					aGradient.EndColor = (sal_Int32)( aColor.GetColor() );
			}
			break;
		case XML_TOK_GRADIENT_STARTINT:
			SvXMLUnitConverter::convertPercent( nTmpValue, rStrValue );
			aGradient.StartIntensity = static_cast< sal_Int16 >( nTmpValue );
			break;
		case XML_TOK_GRADIENT_ENDINT:
			SvXMLUnitConverter::convertPercent( nTmpValue, rStrValue );
			aGradient.EndIntensity = static_cast< sal_Int16 >( nTmpValue );
			break;
		case XML_TOK_GRADIENT_ANGLE:
			{
				sal_Int32 nValue;
				SvXMLUnitConverter::convertNumber( nValue, rStrValue, 0, 3600 );
				aGradient.Angle = sal_Int16( nValue );
			}
			break;
		case XML_TOK_GRADIENT_BORDER:
			SvXMLUnitConverter::convertPercent( nTmpValue, rStrValue );
			aGradient.Border = static_cast< sal_Int16 >( nTmpValue );
			break;

		default:
			DBG_WARNING( "Unknown token at import gradient style" )
			;
		}
	}

	rValue <<= aGradient;

	if( aDisplayName.getLength() )
	{
		rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_GRADIENT_ID, rStrName, 
									 aDisplayName );
		rStrName = aDisplayName;
	}

	bRet = bHasName && bHasStyle && bHasStartColor && bHasEndColor;

    }

	return bRet;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

#ifndef SVX_LIGHT

XMLGradientStyleExport::XMLGradientStyleExport( 
    SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLGradientStyleExport::~XMLGradientStyleExport()
{
}

sal_Bool XMLGradientStyleExport::exportXML( 
    const OUString& rStrName, 
    const uno::Any& rValue )
{
	sal_Bool bRet = sal_False;
	awt::Gradient aGradient;

	if( rStrName.getLength() )
	{
		if( rValue >>= aGradient )
		{
			OUString aStrValue;
			OUStringBuffer aOut;

			// Style
			if( !SvXMLUnitConverter::convertEnum( aOut, aGradient.Style, pXML_GradientStyle_Enum ) )
			{
				bRet = sal_False;
			}
			else
			{
				// Name
				sal_Bool bEncoded = sal_False;
				OUString aStrName( rStrName );
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, 
									  rExport.EncodeStyleName( aStrName,
										 					   &bEncoded ) );
				if( bEncoded )
					rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, 
									  	  aStrName );

				
				aStrValue = aOut.makeStringAndClear();
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );
				
				// Center x/y
				if( aGradient.Style != awt::GradientStyle_LINEAR &&
					aGradient.Style != awt::GradientStyle_AXIAL   )
				{
					SvXMLUnitConverter::convertPercent( aOut, aGradient.XOffset );
					aStrValue = aOut.makeStringAndClear();
					rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CX, aStrValue );
					
					SvXMLUnitConverter::convertPercent( aOut, aGradient.YOffset );
					aStrValue = aOut.makeStringAndClear();
					rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CY, aStrValue );
				}
				
				Color aColor;
				
				// Color start
				aColor.SetColor( aGradient.StartColor );
				SvXMLUnitConverter::convertColor( aOut, aColor );
				aStrValue = aOut.makeStringAndClear();
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_COLOR, aStrValue );
				
				// Color end
				aColor.SetColor( aGradient.EndColor );
				SvXMLUnitConverter::convertColor( aOut, aColor );
				aStrValue = aOut.makeStringAndClear();
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_COLOR, aStrValue );
				
				// Intensity start
				SvXMLUnitConverter::convertPercent( aOut, aGradient.StartIntensity );
				aStrValue = aOut.makeStringAndClear();
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_INTENSITY, aStrValue );
				
				// Intensity end
				SvXMLUnitConverter::convertPercent( aOut, aGradient.EndIntensity );
				aStrValue = aOut.makeStringAndClear();
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_INTENSITY, aStrValue );
				
				// Angle
				if( aGradient.Style != awt::GradientStyle_RADIAL )
				{
					SvXMLUnitConverter::convertNumber( aOut, sal_Int32( aGradient.Angle ) );
					aStrValue = aOut.makeStringAndClear();
					rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, aStrValue );
				}
				
				// Border
				SvXMLUnitConverter::convertPercent( aOut, aGradient.Border );
				aStrValue = aOut.makeStringAndClear();
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, aStrValue );

				// Do Write
				SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_GRADIENT,
                                      sal_True, sal_False );
			}
		}
	}

	return bRet;
}

#endif // #ifndef SVX_LIGHT
