/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef MATRIXSOLVER_H
#define MATRIXSOLVER_H

#include <vector>

    // info about each row/column of the matrix for simplification purposes
class RowInfo 
{
    public:
        RowInfo() { type = ROW_NORMAL; }
        
        static const int ROW_NORMAL = 0;  // ordinary value
        static const int ROW_CONST  = 1;  // value is constant
        static const int ROW_EQUAL  = 2;  // value is equal to another value
        
        int nodeEq, type, mapCol, mapRow;
        double value;
        
        bool rsChanges; // row's right side changes
        bool lsChanges; // row's left side changes
        bool dropRow;   // row is not needed in matrix
        
};

class MatrixSolver
{
    public:
        MatrixSolver();
        ~MatrixSolver();
        
        void simplifyMatrix( std::vector<std::vector<double> > circuitMatrix, std::vector<double>  circuitRightSide );
        
};


 #endif
