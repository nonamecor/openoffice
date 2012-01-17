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
#include "precompiled_basegfx.hxx"
#include <cstdio>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/drawing/LineCap.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
	namespace tools
	{
		B2DPolyPolygon createAreaGeometryForLineStartEnd(
			const B2DPolygon& rCandidate, 
			const B2DPolyPolygon& rArrow, 
			bool bStart,
			double fWidth,
			double fCandidateLength,
			double fDockingPosition, // 0->top, 1->bottom
			double* pConsumedLength)
		{
			B2DPolyPolygon aRetval;
			OSL_ENSURE(rCandidate.count() > 1L, "createAreaGeometryForLineStartEnd: Line polygon has too less points (!)");
			OSL_ENSURE(rArrow.count() > 0L, "createAreaGeometryForLineStartEnd: Empty arrow PolyPolygon (!)");
			OSL_ENSURE(fWidth > 0.0, "createAreaGeometryForLineStartEnd: Width too small (!)");
			OSL_ENSURE(fDockingPosition >= 0.0 && fDockingPosition <= 1.0, 
				"createAreaGeometryForLineStartEnd: fDockingPosition out of range [0.0 .. 1.0] (!)");

            if(fWidth < 0.0)
            {
                fWidth = -fWidth;
            }

            if(rCandidate.count() > 1 && rArrow.count() && !fTools::equalZero(fWidth))
            {
                if(fDockingPosition < 0.0)
                {
                    fDockingPosition = 0.0;
                }
                else if(fDockingPosition > 1.0)
                {
                    fDockingPosition = 1.0;
                }

			    // init return value from arrow
			    aRetval.append(rArrow);

			    // get size of the arrow
			    const B2DRange aArrowSize(getRange(rArrow));

			    // build ArrowTransform; center in X, align with axis in Y
                B2DHomMatrix aArrowTransform(basegfx::tools::createTranslateB2DHomMatrix(
                    -aArrowSize.getCenter().getX(), -aArrowSize.getMinimum().getY()));

			    // scale to target size
			    const double fArrowScale(fWidth / (aArrowSize.getRange().getX()));
			    aArrowTransform.scale(fArrowScale, fArrowScale);

			    // get arrow size in Y
			    B2DPoint aUpperCenter(aArrowSize.getCenter().getX(), aArrowSize.getMaximum().getY());
			    aUpperCenter *= aArrowTransform;
			    const double fArrowYLength(B2DVector(aUpperCenter).getLength());

			    // move arrow to have docking position centered
			    aArrowTransform.translate(0.0, -fArrowYLength * fDockingPosition);

			    // prepare polygon length
			    if(fTools::equalZero(fCandidateLength))
			    {
				    fCandidateLength = getLength(rCandidate);
			    }

			    // get the polygon vector we want to plant this arrow on
			    const double fConsumedLength(fArrowYLength * (1.0 - fDockingPosition));
			    const B2DVector aHead(rCandidate.getB2DPoint((bStart) ? 0L : rCandidate.count() - 1L));
			    const B2DVector aTail(getPositionAbsolute(rCandidate, 
				    (bStart) ? fConsumedLength : fCandidateLength - fConsumedLength, fCandidateLength));

			    // from that vector, take the needed rotation and add rotate for arrow to transformation
			    const B2DVector aTargetDirection(aHead - aTail);
			    const double fRotation(atan2(aTargetDirection.getY(), aTargetDirection.getX()) + (90.0 * F_PI180));

			    // rotate around docking position
			    aArrowTransform.rotate(fRotation);

			    // move arrow docking position to polygon head
			    aArrowTransform.translate(aHead.getX(), aHead.getY());

			    // transform retval and close
			    aRetval.transform(aArrowTransform);
			    aRetval.setClosed(true);

			    // if pConsumedLength is asked for, fill it
			    if(pConsumedLength)
			    {
				    *pConsumedLength = fConsumedLength;
			    }
            }

			return aRetval;
		}
	} // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
	// anonymus namespace for local helpers
	namespace
	{
        bool impIsSimpleEdge(const B2DCubicBezier& rCandidate, double fMaxCosQuad, double fMaxPartOfEdgeQuad)
        {
            // isBezier() is true, already tested by caller
            const B2DVector aEdge(rCandidate.getEndPoint() - rCandidate.getStartPoint());

            if(aEdge.equalZero())
            {
                // start and end point the same, but control vectors used -> baloon curve loop
                // is not a simple edge
                return false;
            }

            // get tangentA and scalar with edge
            const B2DVector aTangentA(rCandidate.getTangent(0.0));
    		const double fScalarAE(aEdge.scalar(aTangentA));

            if(fTools::lessOrEqual(fScalarAE, 0.0))
            {
                // angle between TangentA and Edge is bigger or equal 90 degrees
                return false;
            }

            // get self-scalars for E and A
    		const double fScalarE(aEdge.scalar(aEdge));
    		const double fScalarA(aTangentA.scalar(aTangentA));
			const double fLengthCompareE(fScalarE * fMaxPartOfEdgeQuad);

			if(fTools::moreOrEqual(fScalarA, fLengthCompareE))
			{
				// length of TangentA is more than fMaxPartOfEdge of length of edge
				return false;
			}

            if(fTools::lessOrEqual(fScalarAE * fScalarAE, fScalarA * fScalarE * fMaxCosQuad))
            {
                // angle between TangentA and Edge is bigger or equal angle defined by fMaxCos
                return false;
            }

            // get tangentB and scalar with edge
            const B2DVector aTangentB(rCandidate.getTangent(1.0));
    		const double fScalarBE(aEdge.scalar(aTangentB));

            if(fTools::lessOrEqual(fScalarBE, 0.0))
            {
                // angle between TangentB and Edge is bigger or equal 90 degrees
                return false;
            }

            // get self-scalar for B
    		const double fScalarB(aTangentB.scalar(aTangentB));

			if(fTools::moreOrEqual(fScalarB, fLengthCompareE))
			{
				// length of TangentB is more than fMaxPartOfEdge of length of edge
				return false;
			}

			if(fTools::lessOrEqual(fScalarBE * fScalarBE, fScalarB * fScalarE * fMaxCosQuad))
            {
                // angle between TangentB and Edge is bigger or equal defined by fMaxCos
                return false;
            }

            return true;
        }

        void impSubdivideToSimple(const B2DCubicBezier& rCandidate, B2DPolygon& rTarget, double fMaxCosQuad, double fMaxPartOfEdgeQuad, sal_uInt32 nMaxRecursionDepth)
        {
            if(!nMaxRecursionDepth || impIsSimpleEdge(rCandidate, fMaxCosQuad, fMaxPartOfEdgeQuad))
            {
                rTarget.appendBezierSegment(rCandidate.getControlPointA(), rCandidate.getControlPointB(), rCandidate.getEndPoint());
            }
            else
            {
                B2DCubicBezier aLeft, aRight;
                rCandidate.split(0.5, &aLeft, &aRight);
        
                impSubdivideToSimple(aLeft, rTarget, fMaxCosQuad, fMaxPartOfEdgeQuad, nMaxRecursionDepth - 1);
                impSubdivideToSimple(aRight, rTarget, fMaxCosQuad, fMaxPartOfEdgeQuad, nMaxRecursionDepth - 1);
            }
        }

        B2DPolygon subdivideToSimple(const B2DPolygon& rCandidate, double fMaxCosQuad, double fMaxPartOfEdgeQuad)
        {
            const sal_uInt32 nPointCount(rCandidate.count());

            if(rCandidate.areControlPointsUsed() && nPointCount)
            {
                const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount : nPointCount - 1);
                B2DPolygon aRetval;
				B2DCubicBezier aEdge;

				// prepare edge for loop
				aEdge.setStartPoint(rCandidate.getB2DPoint(0));
				aRetval.append(aEdge.getStartPoint());

                for(sal_uInt32 a(0); a < nEdgeCount; a++)
                {
                    // fill B2DCubicBezier
                    const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                    aEdge.setControlPointA(rCandidate.getNextControlPoint(a));
                    aEdge.setControlPointB(rCandidate.getPrevControlPoint(nNextIndex));
                    aEdge.setEndPoint(rCandidate.getB2DPoint(nNextIndex));
                    
                    // get rid of unnecessary bezier segments
                    aEdge.testAndSolveTrivialBezier();

                    if(aEdge.isBezier())
                    {
						// before splitting recursively with internal simple criteria, use
						// ExtremumPosFinder to remove those
                		::std::vector< double > aExtremumPositions;

                        aExtremumPositions.reserve(4);
		                aEdge.getAllExtremumPositions(aExtremumPositions);
                		
                        const sal_uInt32 nCount(aExtremumPositions.size());

                        if(nCount)
                        {
                            if(nCount > 1)
                            {
                                // create order from left to right
                                ::std::sort(aExtremumPositions.begin(), aExtremumPositions.end());
                            }

                            for(sal_uInt32 b(0); b < nCount;)
                            {
                                // split aEdge at next split pos
                                B2DCubicBezier aLeft;
                                const double fSplitPos(aExtremumPositions[b++]);
                                
                                aEdge.split(fSplitPos, &aLeft, &aEdge);
                                aLeft.testAndSolveTrivialBezier();

                                // consume left part
                                if(aLeft.isBezier())
						        {
	                                impSubdivideToSimple(aLeft, aRetval, fMaxCosQuad, fMaxPartOfEdgeQuad, 6);
						        }
						        else
						        {
	                                aRetval.append(aLeft.getEndPoint());
						        }
                            
                                if(b < nCount)
                                {
                                    // correct the remaining split positions to fit to shortened aEdge
                                    const double fScaleFactor(1.0 / (1.0 - fSplitPos));

                                    for(sal_uInt32 c(b); c < nCount; c++)
                                    {
                                        aExtremumPositions[c] = (aExtremumPositions[c] - fSplitPos) * fScaleFactor;
                                    }
                                }
                            }
                        
                            // test the shortened rest of aEdge
                            aEdge.testAndSolveTrivialBezier();

                            // consume right part
                            if(aEdge.isBezier())
					        {
                                impSubdivideToSimple(aEdge, aRetval, fMaxCosQuad, fMaxPartOfEdgeQuad, 6);
					        }
					        else
					        {
                                aRetval.append(aEdge.getEndPoint());
					        }
                        }
                        else
                        {
	                        impSubdivideToSimple(aEdge, aRetval, fMaxCosQuad, fMaxPartOfEdgeQuad, 6);
                        }
                    }
                    else
                    {
                        // straight edge, add point
                        aRetval.append(aEdge.getEndPoint());
                    }

					// prepare edge for next step
					aEdge.setStartPoint(aEdge.getEndPoint());
                }

				// copy closed flag and check for double points
                aRetval.setClosed(rCandidate.isClosed());
                aRetval.removeDoublePoints();

                return aRetval;
            }
            else
            {
                return rCandidate;
            }
        }

        B2DPolyPolygon createAreaGeometryForEdge(
            const B2DCubicBezier& rEdge, 
            double fHalfLineWidth,
            bool bStartRound,
            bool bEndRound,
            bool bStartSquare,
            bool bEndSquare)
        {
            // create polygon for edge
            // Unfortunately, while it would be geometrically correct to not add
            // the in-between points EdgeEnd and EdgeStart, it leads to rounding
            // errors when converting to integer polygon coordinates for painting
            if(rEdge.isBezier())
            {
                // prepare target and data common for upper and lower
                B2DPolyPolygon aRetval;
                B2DPolygon aBezierPolygon;
                const B2DVector aPureEdgeVector(rEdge.getEndPoint() - rEdge.getStartPoint());
                const double fEdgeLength(aPureEdgeVector.getLength());
                const bool bIsEdgeLengthZero(fTools::equalZero(fEdgeLength));
                B2DVector aTangentA(rEdge.getTangent(0.0)); aTangentA.normalize();
                B2DVector aTangentB(rEdge.getTangent(1.0)); aTangentB.normalize();
                const B2DVector aNormalizedPerpendicularA(getPerpendicular(aTangentA));
                const B2DVector aNormalizedPerpendicularB(getPerpendicular(aTangentB));

                // create upper displacement vectors and check if they cut
                const B2DVector aPerpendStartA(aNormalizedPerpendicularA * -fHalfLineWidth);
                const B2DVector aPerpendEndA(aNormalizedPerpendicularB * -fHalfLineWidth);
                double fCutA(0.0);
                const tools::CutFlagValue aCutA(tools::findCut(
                    rEdge.getStartPoint(), aPerpendStartA,
                    rEdge.getEndPoint(), aPerpendEndA, 
                    CUTFLAG_ALL, &fCutA));
                const bool bCutA(CUTFLAG_NONE != aCutA);

                // create lower displacement vectors and check if they cut
                const B2DVector aPerpendStartB(aNormalizedPerpendicularA * fHalfLineWidth);
                const B2DVector aPerpendEndB(aNormalizedPerpendicularB * fHalfLineWidth);
                double fCutB(0.0);
                const tools::CutFlagValue aCutB(tools::findCut(
                    rEdge.getEndPoint(), aPerpendEndB,
                    rEdge.getStartPoint(), aPerpendStartB, 
                    CUTFLAG_ALL, &fCutB));
                const bool bCutB(CUTFLAG_NONE != aCutB);

                // check if cut happens
                const bool bCut(bCutA || bCutB);

                // create left edge
                if(bStartRound || bStartSquare)
                {
                    basegfx::B2DPolygon aStartPolygon;

                    if(bStartRound)
                    {
                        aStartPolygon = tools::createHalfUnitCircle();
                        aStartPolygon.transform(
                            tools::createScaleShearXRotateTranslateB2DHomMatrix(
                                fHalfLineWidth, fHalfLineWidth,
                                0.0,
                                atan2(aTangentA.getY(), aTangentA.getX()) + F_PI2,
                                rEdge.getStartPoint().getX(), rEdge.getStartPoint().getY()));
                    }
                    else // bStartSquare
                    {
                        const basegfx::B2DPoint aStart(rEdge.getStartPoint() - (aTangentA * fHalfLineWidth));

                        if(bCut)
                        {
                            aStartPolygon.append(rEdge.getStartPoint() + aPerpendStartB);
                        }

                        aStartPolygon.append(aStart + aPerpendStartB);
                        aStartPolygon.append(aStart + aPerpendStartA);

                        if(bCut)
                        {
                            aStartPolygon.append(rEdge.getStartPoint() + aPerpendStartA);
                        }
                    }

                    if(bCut)
                    {
                        aStartPolygon.append(rEdge.getStartPoint());
                        aStartPolygon.setClosed(true);
                        aRetval.append(aStartPolygon);
                    }
                    else
                    {
                        aBezierPolygon.append(aStartPolygon);
                    }
                }
                else
                {
                    // append original in-between point
                    aBezierPolygon.append(rEdge.getStartPoint());
                }

                // create upper edge.
                {
                    if(bCutA)
                    {
                        // calculate cut point and add
                        const B2DPoint aCutPoint(rEdge.getStartPoint() + (aPerpendStartA * fCutA));
                        aBezierPolygon.append(aCutPoint);
                    }
                    else
                    {
                        // create scaled bezier segment
                        const B2DPoint aStart(rEdge.getStartPoint() + aPerpendStartA);
                        const B2DPoint aEnd(rEdge.getEndPoint() + aPerpendEndA);
                        const B2DVector aEdge(aEnd - aStart);
                        const double fLength(aEdge.getLength());
                        const double fScale(bIsEdgeLengthZero ? 1.0 : fLength / fEdgeLength);
                        const B2DVector fRelNext(rEdge.getControlPointA() - rEdge.getStartPoint());
                        const B2DVector fRelPrev(rEdge.getControlPointB() - rEdge.getEndPoint());

                        aBezierPolygon.append(aStart);
                        aBezierPolygon.appendBezierSegment(aStart + (fRelNext * fScale), aEnd + (fRelPrev * fScale), aEnd);
                    }
                }

                // create right edge
                if(bEndRound || bEndSquare)
                {
                    basegfx::B2DPolygon aEndPolygon;

                    if(bEndRound)
                    {
                        aEndPolygon = tools::createHalfUnitCircle();
                        aEndPolygon.transform(
                            tools::createScaleShearXRotateTranslateB2DHomMatrix(
                                fHalfLineWidth, fHalfLineWidth,
                                0.0,
                                atan2(aTangentB.getY(), aTangentB.getX()) - F_PI2,
                                rEdge.getEndPoint().getX(), rEdge.getEndPoint().getY()));
                    }
                    else // bEndSquare
                    {
                        const basegfx::B2DPoint aEnd(rEdge.getEndPoint() + (aTangentB * fHalfLineWidth));

                        if(bCut)
                        {
                            aEndPolygon.append(rEdge.getEndPoint() + aPerpendEndA);
                        }

                        aEndPolygon.append(aEnd + aPerpendEndA);
                        aEndPolygon.append(aEnd + aPerpendEndB);

                        if(bCut)
                        {
                            aEndPolygon.append(rEdge.getEndPoint() + aPerpendEndB);
                        }
                    }

                    if(bCut)
                    {
                        aEndPolygon.append(rEdge.getEndPoint());
                        aEndPolygon.setClosed(true);
                        aRetval.append(aEndPolygon);
                    }
                    else
                    {
                        aBezierPolygon.append(aEndPolygon);
                    }
                }
                else
                {
                    // append original in-between point
                    aBezierPolygon.append(rEdge.getEndPoint());
                }

                // create lower edge. 
                {
                    if(bCutB)
                    {
                        // calculate cut point and add
                        const B2DPoint aCutPoint(rEdge.getEndPoint() + (aPerpendEndB * fCutB));
                        aBezierPolygon.append(aCutPoint);
                    }
                    else
                    {
                        // create scaled bezier segment
                        const B2DPoint aStart(rEdge.getEndPoint() + aPerpendEndB);
                        const B2DPoint aEnd(rEdge.getStartPoint() + aPerpendStartB);
                        const B2DVector aEdge(aEnd - aStart);
                        const double fLength(aEdge.getLength());
                        const double fScale(bIsEdgeLengthZero ? 1.0 : fLength / fEdgeLength);
                        const B2DVector fRelNext(rEdge.getControlPointB() - rEdge.getEndPoint());
                        const B2DVector fRelPrev(rEdge.getControlPointA() - rEdge.getStartPoint());

                        aBezierPolygon.append(aStart);
                        aBezierPolygon.appendBezierSegment(aStart + (fRelNext * fScale), aEnd + (fRelPrev * fScale), aEnd);
                    }
                }

                // close and return
                aBezierPolygon.setClosed(true);
                aRetval.append(aBezierPolygon);
                
                return aRetval;
            }
            else
            {
                // Get start and  end point, create tangent and set to needed length
                B2DVector aTangent(rEdge.getEndPoint() - rEdge.getStartPoint());
                aTangent.setLength(fHalfLineWidth);
                
                // prepare return value
                B2DPolygon aEdgePolygon;

                // buffered angle
                double fAngle(0.0);
                bool bAngle(false);

                // buffered perpendicular
                B2DVector aPerpend;
                bool bPerpend(false);

                // create left vertical
                if(bStartRound)
                {
                    aEdgePolygon = tools::createHalfUnitCircle();
                    fAngle = atan2(aTangent.getY(), aTangent.getX());
                    bAngle = true;
                    aEdgePolygon.transform(
                        tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            fHalfLineWidth, fHalfLineWidth,
                            0.0,
                            fAngle + F_PI2,
                            rEdge.getStartPoint().getX(), rEdge.getStartPoint().getY()));
                }
                else
                {
                    aPerpend.setX(-aTangent.getY());
                    aPerpend.setY(aTangent.getX());
                    bPerpend = true;

                    if(bStartSquare)
                    {
                        const basegfx::B2DPoint aStart(rEdge.getStartPoint() - aTangent);

                        aEdgePolygon.append(aStart + aPerpend);
                        aEdgePolygon.append(aStart - aPerpend);
                    }
                    else
                    {
                        aEdgePolygon.append(rEdge.getStartPoint() + aPerpend);
                        aEdgePolygon.append(rEdge.getStartPoint()); // keep the in-between point for numerical reasons
                        aEdgePolygon.append(rEdge.getStartPoint() - aPerpend);
                    }
                }

                // create right vertical
                if(bEndRound)
                {
                    basegfx::B2DPolygon aEndPolygon(tools::createHalfUnitCircle());

                    if(!bAngle)
                    {
                        fAngle = atan2(aTangent.getY(), aTangent.getX());
                    }

                    aEndPolygon.transform(
                        tools::createScaleShearXRotateTranslateB2DHomMatrix(
                            fHalfLineWidth, fHalfLineWidth,
                            0.0,
                            fAngle - F_PI2,
                            rEdge.getEndPoint().getX(), rEdge.getEndPoint().getY()));
                    aEdgePolygon.append(aEndPolygon);
                }
                else
                {
                    if(!bPerpend)
                    {
                        aPerpend.setX(-aTangent.getY());
                        aPerpend.setY(aTangent.getX());
                    }

                    if(bEndSquare)
                    {
                        const basegfx::B2DPoint aEnd(rEdge.getEndPoint() + aTangent);

                        aEdgePolygon.append(aEnd - aPerpend);
                        aEdgePolygon.append(aEnd + aPerpend);
                    }
                    else
                    {
                        aEdgePolygon.append(rEdge.getEndPoint() - aPerpend);
                        aEdgePolygon.append(rEdge.getEndPoint()); // keep the in-between point for numerical reasons
                        aEdgePolygon.append(rEdge.getEndPoint() + aPerpend);
                    }
                }

                // close and return
                aEdgePolygon.setClosed(true);

                return B2DPolyPolygon(aEdgePolygon);
            }
        }

        B2DPolygon createAreaGeometryForJoin(
            const B2DVector& rTangentPrev, 
            const B2DVector& rTangentEdge,
            const B2DVector& rPerpendPrev, 
            const B2DVector& rPerpendEdge,
            const B2DPoint& rPoint,
            double fHalfLineWidth, 
            B2DLineJoin eJoin, 
            double fMiterMinimumAngle)
		{
			OSL_ENSURE(fHalfLineWidth > 0.0, "createAreaGeometryForJoin: LineWidth too small (!)");
			OSL_ENSURE(B2DLINEJOIN_NONE != eJoin, "createAreaGeometryForJoin: B2DLINEJOIN_NONE not allowed (!)");

            // LineJoin from tangent rPerpendPrev to tangent rPerpendEdge in rPoint
            B2DPolygon aEdgePolygon;
			const B2DPoint aStartPoint(rPoint + rPerpendPrev);
			const B2DPoint aEndPoint(rPoint + rPerpendEdge);
            
			// test if for Miter, the angle is too small and the fallback
			// to bevel needs to be used
			if(B2DLINEJOIN_MITER == eJoin)
			{
				const double fAngle(fabs(rPerpendPrev.angle(rPerpendEdge)));

				if((F_PI - fAngle) < fMiterMinimumAngle)
				{
					// fallback to bevel
					eJoin = B2DLINEJOIN_BEVEL;
				}
			}

			switch(eJoin)
			{
				case B2DLINEJOIN_MITER :
				{
					aEdgePolygon.append(aEndPoint);
					aEdgePolygon.append(rPoint);
					aEdgePolygon.append(aStartPoint);
					
					// Look for the cut point between start point along rTangentPrev and
					// end point along rTangentEdge. -rTangentEdge should be used, but since
					// the cut value is used for interpolating along the first edge, the negation
					// is not needed since the same fCut will be found on the first edge.
					// If it exists, insert it to complete the mitered fill polygon.
    				double fCutPos(0.0);
					tools::findCut(aStartPoint, rTangentPrev, aEndPoint, rTangentEdge, CUTFLAG_ALL, &fCutPos);

					if(0.0 != fCutPos)
					{
						const B2DPoint aCutPoint(interpolate(aStartPoint, aStartPoint + rTangentPrev, fCutPos));
						aEdgePolygon.append(aCutPoint);
					}
					
					break;
				}
				case B2DLINEJOIN_ROUND :
				{
					// use tooling to add needed EllipseSegment
					double fAngleStart(atan2(rPerpendPrev.getY(), rPerpendPrev.getX()));
					double fAngleEnd(atan2(rPerpendEdge.getY(), rPerpendEdge.getX()));
	                
					// atan2 results are [-PI .. PI], consolidate to [0.0 .. 2PI]
					if(fAngleStart < 0.0)
					{
						fAngleStart += F_2PI;
					}

					if(fAngleEnd < 0.0)
					{
						fAngleEnd += F_2PI;
					}

					const B2DPolygon aBow(tools::createPolygonFromEllipseSegment(rPoint, fHalfLineWidth, fHalfLineWidth, fAngleStart, fAngleEnd));

					if(aBow.count() > 1)
					{
						// #i101491#
						// use the original start/end positions; the ones from bow creation may be numerically
						// different due to their different creation. To guarantee good merging quality with edges
						// and edge roundings (and to reduce point count)
						aEdgePolygon = aBow;
						aEdgePolygon.setB2DPoint(0, aStartPoint);
						aEdgePolygon.setB2DPoint(aEdgePolygon.count() - 1, aEndPoint);
						aEdgePolygon.append(rPoint);

						break;
					}
					else
					{
						// wanted fall-through to default
					}
				}
				default: // B2DLINEJOIN_BEVEL
				{
					aEdgePolygon.append(aEndPoint);
					aEdgePolygon.append(rPoint);
					aEdgePolygon.append(aStartPoint);
					
					break;
				}
			}

            // create last polygon part for edge
            aEdgePolygon.setClosed(true);

            return aEdgePolygon;
        }
    } // end of anonymus namespace

	namespace tools
	{
        B2DPolyPolygon createAreaGeometry(
            const B2DPolygon& rCandidate, 
            double fHalfLineWidth, 
            B2DLineJoin eJoin, 
            com::sun::star::drawing::LineCap eCap,
            double fMaxAllowedAngle, 
			double fMaxPartOfEdge,
            double fMiterMinimumAngle)
		{
            if(fMaxAllowedAngle > F_PI2)
            {
                fMaxAllowedAngle = F_PI2;
            }
            else if(fMaxAllowedAngle < 0.01 * F_PI2)
            {
                fMaxAllowedAngle = 0.01 * F_PI2;
            }

            if(fMaxPartOfEdge > 1.0)
            {
                fMaxPartOfEdge = 1.0;
            }
            else if(fMaxPartOfEdge < 0.01)
            {
                fMaxPartOfEdge = 0.01;
            }

            if(fMiterMinimumAngle > F_PI)
            {
                fMiterMinimumAngle = F_PI;
            }
            else if(fMiterMinimumAngle < 0.01 * F_PI)
            {
                fMiterMinimumAngle = 0.01 * F_PI;
            }

            B2DPolygon aCandidate(rCandidate);
            const double fMaxCos(cos(fMaxAllowedAngle));
            
            aCandidate.removeDoublePoints();
            aCandidate = subdivideToSimple(aCandidate, fMaxCos * fMaxCos, fMaxPartOfEdge * fMaxPartOfEdge);
	        
            const sal_uInt32 nPointCount(aCandidate.count());

			if(nPointCount)
			{
                B2DPolyPolygon aRetval;
				const bool bEventuallyCreateLineJoin(B2DLINEJOIN_NONE != eJoin);
                const bool bIsClosed(aCandidate.isClosed());
                const sal_uInt32 nEdgeCount(bIsClosed ? nPointCount : nPointCount - 1);
                const bool bLineCap(!bIsClosed && com::sun::star::drawing::LineCap_BUTT != eCap);

                if(nEdgeCount)
                {
                    B2DCubicBezier aEdge;
                    B2DCubicBezier aPrev;

                    // prepare edge
                    aEdge.setStartPoint(aCandidate.getB2DPoint(0));

                    if(bIsClosed && bEventuallyCreateLineJoin)
                    {
                        // prepare previous edge
                        const sal_uInt32 nPrevIndex(nPointCount - 1);
                        aPrev.setStartPoint(aCandidate.getB2DPoint(nPrevIndex));
                        aPrev.setControlPointA(aCandidate.getNextControlPoint(nPrevIndex));
                        aPrev.setControlPointB(aCandidate.getPrevControlPoint(0));
                        aPrev.setEndPoint(aEdge.getStartPoint());
                    }

                    for(sal_uInt32 a(0); a < nEdgeCount; a++)
                    {
                        // fill current Edge
                        const sal_uInt32 nNextIndex((a + 1) % nPointCount);
                        aEdge.setControlPointA(aCandidate.getNextControlPoint(a));
                        aEdge.setControlPointB(aCandidate.getPrevControlPoint(nNextIndex));
                        aEdge.setEndPoint(aCandidate.getB2DPoint(nNextIndex));

                        // check and create linejoin
                        if(bEventuallyCreateLineJoin && (bIsClosed || 0 != a))
                        {
                            B2DVector aTangentPrev(aPrev.getTangent(1.0)); aTangentPrev.normalize();
                            B2DVector aTangentEdge(aEdge.getTangent(0.0)); aTangentEdge.normalize();
                            B2VectorOrientation aOrientation(getOrientation(aTangentPrev, aTangentEdge));

                            if(ORIENTATION_NEUTRAL == aOrientation)
                            {
                                   // they are parallell or empty; if they are both not zero and point 
                                   // in opposite direction, a half-circle is needed
                                   if(!aTangentPrev.equalZero() && !aTangentEdge.equalZero())
                                   {
                                    const double fAngle(fabs(aTangentPrev.angle(aTangentEdge)));
        
                                    if(fTools::equal(fAngle, F_PI))
                                    {
                                        // for half-circle production, fallback to positive
                                        // orientation
                                        aOrientation = ORIENTATION_POSITIVE;
                                    }
                                }
                            }

                            if(ORIENTATION_POSITIVE == aOrientation)
                            {
                                const B2DVector aPerpendPrev(getPerpendicular(aTangentPrev) * -fHalfLineWidth);
                                const B2DVector aPerpendEdge(getPerpendicular(aTangentEdge) * -fHalfLineWidth);
                                
                                aRetval.append(
                                    createAreaGeometryForJoin(
                                        aTangentPrev, 
                                        aTangentEdge,
                                        aPerpendPrev, 
                                        aPerpendEdge, 
                                        aEdge.getStartPoint(), 
                                        fHalfLineWidth, 
                                        eJoin, 
                                        fMiterMinimumAngle));
                            }
                            else if(ORIENTATION_NEGATIVE == aOrientation)
                            {
                                const B2DVector aPerpendPrev(getPerpendicular(aTangentPrev) * fHalfLineWidth);
                                const B2DVector aPerpendEdge(getPerpendicular(aTangentEdge) * fHalfLineWidth);

                                aRetval.append(
                                    createAreaGeometryForJoin(
                                        aTangentEdge, 
                                        aTangentPrev,
                                        aPerpendEdge, 
                                        aPerpendPrev, 
                                        aEdge.getStartPoint(), 
                                        fHalfLineWidth, 
                                        eJoin, 
                                        fMiterMinimumAngle));
                            }
                        }

                        // create geometry for edge
                        const bool bLast(a + 1 == nEdgeCount);

                        if(bLineCap)
                        {
                            const bool bFirst(!a);

                            aRetval.append(
                                createAreaGeometryForEdge(
                                    aEdge, 
                                    fHalfLineWidth,
                                    bFirst && com::sun::star::drawing::LineCap_ROUND == eCap,
                                    bLast && com::sun::star::drawing::LineCap_ROUND == eCap,
                                    bFirst && com::sun::star::drawing::LineCap_SQUARE == eCap,
                                    bLast && com::sun::star::drawing::LineCap_SQUARE == eCap));
                        }
                        else
                        {
                            aRetval.append(
                                createAreaGeometryForEdge(
                                    aEdge, 
                                    fHalfLineWidth,
                                    false,
                                    false,
                                    false,
                                    false));
                        }

                        // prepare next step
                        if(!bLast)
                        {
                            if(bEventuallyCreateLineJoin)
                            {
                                aPrev = aEdge;
                            }

                            aEdge.setStartPoint(aEdge.getEndPoint());
                        }
                    }
                }

                return aRetval;
            }
            else
            {
                return B2DPolyPolygon(rCandidate);
            }
        }
    } // end of namespace tools
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
