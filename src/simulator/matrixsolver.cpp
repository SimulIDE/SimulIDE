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
 
#include <iostream>

#include "matrixsolver.h"

using namespace std;


MatrixSolver::MatrixSolver()
{
}
MatrixSolver::~MatrixSolver()
{
}

void MatrixSolver::simplifyMatrix( vector<vector<double> > circuitMatrix, std::vector<double> circuitRightSide )
{
    int matrixSize = circuitMatrix.size();
    RowInfo circuitRowInfo[matrixSize];
    std::cout<< "size = " << matrixSize<<std::endl;
    
    // simplify the matrix; this speeds things up quite a bit
    for( int i = 0; i!=matrixSize; i++) 
    {
        int    qm = -1;
        int    qp = -1;
        double qv = 0;
        
        //RowInfo re = circuitRowInfo[i];
        
        /*System.out.println("row " + i + " " + re.lsChanges + " " + re.rsChanges + " " + re.dropRow);*/
        
        //if (re.lsChanges || re.dropRow || re.rsChanges) continue;
        
        double rsadd = 0;
        int j;
        // look for rows that can be removed
        for( j=0; j!=matrixSize; j++ ) 
        {
            double q = circuitMatrix[i][j];
            
            if( circuitRowInfo[j].type == RowInfo::ROW_CONST ) 
            {
                // keep a running total of const values that have been
                // removed already
                rsadd -= circuitRowInfo[j].value*q;
                continue;
            }
            //std::cout<< "Row "<< i << "col" <<j<< " element = " << q <<std::endl;
            if (q == 0) continue;
            
            if (qp == -1)                             // First Non Ceros
            {
                qp = j;
                qv = q;
                continue;
            }
            if (qm == -1 && q == -qv)                 // Second Non Cero
            {
                qm = j;
                continue;
            }
            break;
        }
        //System.out.println("line " + i + " " + qp + " " + qm + " " + j);
        /*if (qp != -1 && circuitRowInfo[qp].lsChanges) {
            System.out.println("lschanges");
            continue;
        }
        if (qm != -1 && circuitRowInfo[qm].lsChanges) {
            System.out.println("lschanges");
            continue;
            }*/
        if( j == matrixSize )                      // 2 or less Non Cero
        {
            if (qp == -1) 
            {
                std::cout<< "Matrix error" << std::endl;
                return;
            }
            RowInfo elt = circuitRowInfo[qp];
            if( qm == -1 )                                 // 1 Non Cero
            {std::cout<< "constant" << std::endl;
                // we found a row with only one nonzero entry; that value
                // is a constant
                for( int k=0; elt.type == RowInfo::ROW_EQUAL && k<100; k++) 
                {
                    // follow the chain
                    /*System.out.println("following equal chain from " +
                                       i + " " + qp + " to " + elt.nodeEq);*/
                    qp = elt.nodeEq;
                    elt = circuitRowInfo[qp];
                }
                if( elt.type == RowInfo::ROW_EQUAL) 
                {
                    // break equal chains
                    //System.out.println("Break equal chain");
                    elt.type = RowInfo::ROW_NORMAL;
                    continue;
                }
                if (elt.type != RowInfo::ROW_NORMAL) 
                {
                    // System.out.println("type already " + elt.type + " for " + qp + "!");
                    continue;
                }
                elt.type = RowInfo::ROW_CONST;
                elt.value = (circuitRightSide[i]+rsadd)/qv;
                circuitRowInfo[i].dropRow = true;
                //System.out.println(qp + " * " + qv + " = const " + elt.value);
                i = -1; // start over from scratch
            } 
            else if (circuitRightSide[i]+rsadd == 0)       // 2 Non Cero
            {std::cout<< "two nonzero entries" << std::endl;
                // we found a row with only two nonzero entries, and one
                // is the negative of the other; the values are equal
                if (elt.type != RowInfo::ROW_NORMAL) 
                {
                    //System.out.println("swapping");
                    int qq = qm;
                    qm = qp; qp = qq;
                    elt = circuitRowInfo[qp];
                    
                    if (elt.type != RowInfo::ROW_NORMAL) 
                    {
                        // we should follow the chain here, but this
                        // hardly ever happens so it's not worth worrying
                        // about
                        std::cout<< "swap failed" << std::endl;
                        continue;
                    }
                }
                elt.type = RowInfo::ROW_EQUAL;
                elt.nodeEq = qm;
                circuitRowInfo[i].dropRow = true;
                //System.out.println(qp + " = " + qm);
            }
        }
    }
    // find size of new matrix
    int nn = 0;
    for( int i=0; i!=matrixSize; i++) 
    {
        RowInfo elt = circuitRowInfo[i];
        std::cout<< "elt.type" << elt.type<<std::endl;
        if (elt.type == RowInfo::ROW_NORMAL) 
        {
            elt.mapCol = nn++;
            //System.out.println("col " + i + " maps to " + elt.mapCol);
            continue;
        }
        if (elt.type == RowInfo::ROW_EQUAL) 
        {
            RowInfo e2;
            // resolve chains of equality; 100 max steps to avoid loops
            for( int j=0; j!=100; j++ ) 
            {
                e2 = circuitRowInfo[elt.nodeEq];
                if( e2.type != RowInfo::ROW_EQUAL ) break;
                if( i == e2.nodeEq )                break;
                elt.nodeEq = e2.nodeEq;
            }
        }
        if( elt.type == RowInfo::ROW_CONST )
            elt.mapCol = -1;
    }
    for( int i=0; i!=matrixSize; i++) 
    {
        RowInfo elt = circuitRowInfo[i];
        if( elt.type == RowInfo::ROW_EQUAL ) 
        {
            RowInfo e2 = circuitRowInfo[elt.nodeEq];
            if( e2.type == RowInfo::ROW_CONST ) 
            {
                // if something is equal to a const, it's a const
                elt.type = e2.type;
                elt.value = e2.value;
                elt.mapCol = -1;
                //System.out.println(i + " = [late]const " + elt.value);
            } 
            else 
            {
                elt.mapCol = e2.mapCol;
                //System.out.println(i + " maps to: " + e2.mapCol);
            }
        }
    }
    //System.out.println("ac8");

    /*System.out.println("matrixSize = " + matrixSize);
    
    for (j = 0; j != circuitMatrixSize; j++) {
        System.out.println(j + ": ");
        for (i = 0; i != circuitMatrixSize; i++)
            System.out.print(circuitMatrix[j][i] + " ");
        System.out.print("  " + circuitRightSide[j] + "\n");
    }
    System.out.print("\n");*/
    
    std::cout << "\nSimplified Matrix:\n"<< std::endl;
    for( int i=0; i<matrixSize; i++ )
    {
        for( int j=0; j<matrixSize; j++ )
        {
            std::cout << circuitMatrix[i][j] <<"\t";
        }
        std::cout << "\t";
        std::cout << circuitRightSide[i]<< std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
    }
    

    // make the new, simplified matrix
    int newsize = nn;
    std::cout << "\nNew size Matrix:\n"<< nn <<std::endl;
    double newmatx[newsize][newsize];
    double newrs  [newsize];
    int ii = 0;
    for( int i=0; i!=matrixSize; i++) 
    {
        RowInfo rri = circuitRowInfo[i];
        if( rri.dropRow ) 
        {
            rri.mapRow = -1;
            continue;
        }
        newrs[ii] = circuitRightSide[i];
        rri.mapRow = ii;
        //System.out.println("Row " + i + " maps to " + ii);
        for( int j=0; j!=matrixSize; j++ ) 
        {
            RowInfo ri = circuitRowInfo[j];
            if (ri.type == RowInfo::ROW_CONST)
                newrs[ii] -= ri.value*circuitMatrix[i][j];
            else
                newmatx[ii][ri.mapCol] += circuitMatrix[i][j];
        }
        ii++;
    }
    
    std::cout << "\nSimplified Matrix:\n"<< std::endl;
    for( int i=0; i<newsize; i++ )
    {
        for( int j=0; j<newsize; j++ )
        {
            std::cout << newmatx[i][j] <<"\t";
        }
        std::cout << "\t";
        std::cout << newrs[i]<< std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
    }
/*
    circuitMatrix = newmatx;
    circuitRightSide = newrs;
    matrixSize = circuitMatrixSize = newsize;
    for (i = 0; i != matrixSize; i++)
        origRightSide[i] = circuitRightSide[i];
    for (i = 0; i != matrixSize; i++)
        for (j = 0; j != matrixSize; j++)
            origMatrix[i][j] = circuitMatrix[i][j];
*/
    //circuitNeedsMap = true;
}
