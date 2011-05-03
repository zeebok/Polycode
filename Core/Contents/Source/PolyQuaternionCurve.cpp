/*
 *  PolyQuaternionCurve.cpp
 *  Poly
 *
 *  Created by Ivan Safrin on 9/15/08.
 *  Copyright 2008 Ivan Safrin. All rights reserved.
 *
 */


#include "PolyQuaternionCurve.h"

using namespace Polycode;

QuaternionCurve::QuaternionCurve(BezierCurve *wCurve, BezierCurve *xCurve, BezierCurve *yCurve, BezierCurve *zCurve) {
	generatePointsFromCurves(wCurve, xCurve, yCurve, zCurve);
}

QuaternionCurve::~QuaternionCurve() {

}

void QuaternionCurve::generatePointsFromCurves(BezierCurve *wCurve, BezierCurve *xCurve, BezierCurve *yCurve, BezierCurve *zCurve) {
	for(int i=0; i < wCurve->getNumControlPoints(); i++) {
		/*
		Quaternion newQuat(wCurve->getControlPoint(i)->p2.y,
							xCurve->getControlPoint(i)->p2.y,
							yCurve->getControlPoint(i)->p2.y,
							zCurve->getControlPoint(i)->p2.y);

		points.push_back(newQuat);
		recalcTangents();
		*/
		Quaternion quat1(wCurve->getControlPoint(i)->p1.y,
							xCurve->getControlPoint(i)->p1.y,
							yCurve->getControlPoint(i)->p1.y,
							zCurve->getControlPoint(i)->p1.y);

		Quaternion quat2(wCurve->getControlPoint(i)->p2.y,
							xCurve->getControlPoint(i)->p2.y,
							yCurve->getControlPoint(i)->p2.y,
							zCurve->getControlPoint(i)->p2.y);

		Quaternion quat3(wCurve->getControlPoint(i)->p3.y,
							xCurve->getControlPoint(i)->p3.y,
							yCurve->getControlPoint(i)->p3.y,
							zCurve->getControlPoint(i)->p3.y);

		
		QuatTriple newTriple;
		newTriple.q1 = quat1;
		newTriple.q2 = quat2;
		newTriple.q3 = quat3;		
		tPoints.push_back(newTriple);
	}
}

    Quaternion QuaternionCurve::interpolate(Number t, bool useShortestPath)
    {
        // Work out which segment this is in
        Number fSeg = t * (tPoints.size() - 1);
        unsigned int segIdx = (unsigned int)fSeg;
        // Apportion t 
        t = fSeg - segIdx;

        return interpolate(segIdx, t, useShortestPath);

    }
    //---------------------------------------------------------------------
    Quaternion QuaternionCurve::interpolate(unsigned int fromIndex, Number t,
		bool useShortestPath)
    {
        // Bounds check
        assert (fromIndex >= 0 && fromIndex < tPoints.size() &&
            "fromIndex out of bounds");

        if ((fromIndex + 1) == tPoints.size() && tPoints[fromIndex].q2 != tPoints[0].q2)
        {
			Logger::log("size\n");
            // Duff request, cannot blend to nothing
            // Just return source
            return points[fromIndex];

        }
        // Fast special cases
        if (t == 0.0f)
        {
            return tPoints[fromIndex].q2;
        }
        else if(t == 1.0f)
        {
            return tPoints[fromIndex + 1].q2;
        }
		
        Quaternion &p = tPoints[fromIndex].q2;
        Quaternion &q = tPoints[fromIndex+1].q2;
        Quaternion &a = tPoints[fromIndex].q3;
        Quaternion &b = tPoints[fromIndex+1].q1;		
		
		if ((fromIndex + 1) == tPoints.size() && tPoints[fromIndex].q2 == tPoints[0].q2) {
			q = tPoints[1].q2;
			b = tPoints[1].q1;					
		} 

        // NB interpolate to nearest rotation
        return Quaternion::Squad(t, p, a, b, q, useShortestPath);

    }

void QuaternionCurve::recalcTangents(void)
{
        unsigned int i, numPoints;
        bool isClosed;

        numPoints = (unsigned int)points.size();

        if (numPoints < 2) {
            return;
        }

        tangents.resize(numPoints);

        if (points[0] == points[numPoints-1])
        {
            isClosed = true;
        }
        else
        {
            isClosed = false;
        }

        Quaternion invp, part1, part2, preExp;
        for(i = 0; i < numPoints; ++i)
        {
            Quaternion &p = points[i];
            invp = p.Inverse();

            if (i ==0)
            {
                // special case start
                part1 = (invp * points[i+1]).Log();
                if (isClosed)
                {
                    // Use numPoints-2 since numPoints-1 == end == start == this one
                    part2 = (invp * points[numPoints-2]).Log();
                }
                else
                {
                    part2 = (invp * p).Log();
                }
            }
            else if (i == numPoints-1)
            {
                // special case end
                if (isClosed)
                {
                    // Wrap to [1] (not [0], this is the same as end == this one)
                    part1 = (invp * points[1]).Log();
                }
                else
                {
                    part1 = (invp * p).Log();
                }
                part2 = (invp * points[i-1]).Log();
            }
            else
            {
                part1 = (invp * points[i+1]).Log();
                part2 = (invp * points[i-1]).Log();
            }

            preExp =(part1 + part2)  * -0.25 ;
            tangents[i] = p * preExp.Exp();
            
        }



    }