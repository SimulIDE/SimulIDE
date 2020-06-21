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
#include <math.h>

//#include <iomanip>

#include "circmatrix.h"
#include "simulator.h"

CircMatrix* CircMatrix::m_pSelf = 0l;

CircMatrix::CircMatrix()
{
    m_pSelf = this;
    m_numEnodes = 0;
}
CircMatrix::~CircMatrix(){}

void CircMatrix::createMatrix( QList<eNode*> &eNodeList )
{
    m_eNodeList = &eNodeList;
    m_numEnodes = eNodeList.size();

    m_circMatrix.clear();
    m_coefVect.clear();

    m_circMatrix.resize( m_numEnodes , d_vector_t( m_numEnodes , 0 ) );
    m_coefVect.resize( m_numEnodes , 0 );
    
    m_circChanged  = true;
    m_admitChanged = false;
    m_currChanged  = false;

    std::cout <<"\n  Initializing Matrix: "<< m_numEnodes << " eNodes"<< std::endl;
    for( int i=0; i<m_numEnodes; i++ ) m_eNodeList->at(i)->stampMatrix();
}

void CircMatrix::stampMatrix( int row, int col, double value )
{
    m_admitChanged = true;
    
    m_circMatrix[row-1][col-1] = value;      // eNode numbers start at 1
}

void CircMatrix::stampCoef( int row, double value )
{
    m_currChanged = true;
    
    m_coefVect[row-1] = value;
}

void CircMatrix::addConnections( int enodNum, QList<int>* nodeGroup, QList<int>* allNodes )
{
    nodeGroup->append( enodNum );
    allNodes->removeOne( enodNum );

    eNode* enod = m_eNodeList->at( enodNum-1 );
    enod->setSingle( false );
    
    QList<int> cons = enod->getConnections();
    
    for( int nodeNum : cons ) 
    {
        if( nodeNum == 0 ) continue;
        if( !nodeGroup->contains( nodeNum ) ) addConnections( nodeNum, nodeGroup, allNodes );
    }
}

bool CircMatrix::solveMatrix()
{
    if( !m_admitChanged && !m_currChanged ) return true;

    bool isOk = true;

    if( m_circChanged )          // Split Circuit into unconnected parts
    {
        //qDebug() <<"Spliting Circuit...";
        QList<int> allNodes;
        
        for( int i=0; i<m_numEnodes; i++ ) allNodes.append( i+1 );
        
        m_aList.clear();
        m_aFaList.clear();
        m_bList.clear();
        m_ipvtList.clear();
        m_eNodeActList.clear();
        int group = 0;
        
        while( !allNodes.isEmpty() ) // Get a list of groups of nodes interconnected
        {
            QList<int> nodeGroup;
            addConnections( allNodes.first(), &nodeGroup, &allNodes ); // Get a group of nodes interconnected
            //qDebug() <<"CircMatrix::solveMatrix split"<<nodeGroup<<allNodes;
            
            //for( int num : nodeGroup ) allNodes.removeOne(num);
            
            int numEnodes = nodeGroup.size();
            if( numEnodes==1 )           // Sigle nodes do by themselves
            {
                eNode* enod = m_eNodeList->at( nodeGroup[0]-1 );
                enod->setSingle( true );
                enod->solveSingle();
                //qDebug() <<"CircMatrix::solveMatrix solve single"<<enod->itemId();
            }
            else
            {
                dp_matrix_t a;
                d_matrix_t ap;
                dp_vector_t b;
                i_vector_t ipvt;
                QList<eNode*> eNodeActive;
                
                a.resize( numEnodes , dp_vector_t( numEnodes , 0 ) );
                ap.resize( numEnodes , d_vector_t( numEnodes , 0 ) );
                b.resize( numEnodes , 0 );
                ipvt.resize( numEnodes , 0 );

                int ny=0;
                for( int y=0; y<m_numEnodes; y++ )    // Copy data to reduced Matrix
                {
                    if( !nodeGroup.contains( y+1 ) ) continue;
                    int nx=0;
                    for( int x=0; x<m_numEnodes; x++ )      
                    {
                        if( !nodeGroup.contains( x+1 ) ) continue;
                        a[nx][ny] = &(m_circMatrix[x][y]);
                        //qDebug() <<"CircMatrix::solveMatrix cell"<<nx<<ny<<*(a[nx][ny]);
                        nx++;
                    }
                    b[ny] = &(m_coefVect[y]);
                    eNodeActive.append( m_eNodeList->at(y) );
                    //eNode* enod = m_eNodeList->at(y);
                    //qDebug() <<"CircMatrix::solveMatrix node"<<enod->itemId();
                    ny++;
                }
                m_aList.append( a );
                m_aFaList.append( ap );
                m_bList.append( b );
                m_ipvtList.append( ipvt );
                m_eNodeActList.append( eNodeActive );
                m_eNodeActive = &eNodeActive;
                
                factorMatrix( ny, group );
                isOk &= luSolve( ny, group );

                group++;
            }
        }
        m_circChanged  = false;
        //qDebug() <<"CircMatrix::solveMatrix"<<group<<"Circuits";
    }
    else
    {
        for( int i=0; i<m_bList.size(); i++ )
        {
            m_eNodeActive = &(m_eNodeActList[i]);
            int n = m_eNodeActive->size();

            if( m_admitChanged ) factorMatrix( n, i );

            isOk &= luSolve( n, i );
        }
    }
    m_currChanged  = false;
    m_admitChanged = false;
    return isOk;
}

void CircMatrix::factorMatrix( int n, int group  )
{
    // factors a matrix into upper and lower triangular matrices by
    // gaussian elimination.  On entry, a[0..n-1][0..n-1] is the
    // matrix to be factored.  ipvt[] returns an integer vector of pivot
    // indices, used in the solve routine.
    
    dp_matrix_t&  ap  = m_aList[group];
    i_vector_t&  ipvt = m_ipvtList[group];
    
    d_matrix_t& a = m_aFaList[group];
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; j++ )
        {
             a[i][j] = *(ap[i][j]);             
             //qDebug() << m_circMatrix[i][j];
        }
    }
    
    /*std::cout << "\nAdmitance Matrix:\n"<< std::endl;
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; j++ )
        {
            std::cout << std::setw(10);
            std::cout << a[i][j];
        }
        std::cout << std::setw(10);
        std::cout << ipvt[i] << std::endl;
        //std::cout << std::endl;
    }*/
    
    int i,j,k;

    for( j=0; j<n; j++ ) // use Crout's method; loop through the columns
    {
        for( i=0; i<j; i++ ) // calculate upper triangular elements for this column
        {
            double q = a[i][j];
            for( k=0; k<i; k++ ) q -= a[i][k]*a[k][j];

            a[i][j] = q;
        }
                           // calculate lower triangular elements for this column
        double largest = 0;
        int largestRow = -1;
        for( i=j; i<n; i++ )
        {
            double q = a[i][j];
            for( k=0; k<j; k++ ) q -= a[i][k]*a[k][j];

            a[i][j] = q;
            double x = std::abs( q );
            
            //qDebug() <<"is"<<x<<">="<<largest<<( x >= largest );
            if( x >= largest )
            {
                largest = x;
                largestRow = i;
            }
            //qDebug() <<"LTE"<<i<<j<<x<<q<<largest<<largestRow<<( !(x < largest) );
        }

        if( j != largestRow ) // pivoting
        {
            double x;
            for( k=0; k<n; k++ )
            {
                x = a[largestRow][k];
                a[largestRow][k] = a[j][k];
                a[j][k] = x;
            }
        }
        ipvt[j] = largestRow;      // keep track of row interchanges
        //qDebug() <<"IPVT"<< j<<largestRow;

        if( a[j][j] == 0.0 ) a[j][j]=1e-18;           // avoid zeros

        if( j != n-1 )
        {
            double div = a[j][j];
            for( i=j+1; i<n; i++ ) a[i][j] /= div;
        }
    }
    m_aFaList.replace( group, a );
    
    /*std::cout << "\nFactored Matrix:\n"<< std::endl;
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; j++ )
        {
            std::cout << std::setw(10);
            std::cout << a[i][j];
        }
        std::cout << std::setw(10);
        std::cout << ipvt[i] << std::endl;
        //std::cout << std::endl;
    }*/
}

bool CircMatrix::luSolve( int n, int group )
{
    // Solves the set of n linear equations using a LU factorization
    // previously performed by solveMatrix.  On input, b[0..n-1] is the right
    // hand side of the equations, and on output, contains the solution.

    const d_matrix_t&  a    = m_aFaList[group];
    const dp_vector_t& bp   = m_bList[group];
    const i_vector_t&  ipvt = m_ipvtList[group];

    d_vector_t b;
    b.resize( n , 0 );
    for( int i=0; i<n; i++ ) b[i] = *(bp[i]);
    
    /*std::cout << "\nAdmitance Matrix luSolve:\n"<< std::endl;
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; j++ )
        {
            std::cout << std::setw(10);
            std::cout << a[i][j]; // <<"\t";
        }
        std::cout << std::setw(10);
        std::cout << b[i]<<"\t"<< ipvt[i] << std::endl;
    }*/

    int i;
    for( i=0; i<n; i++ )                 // find first nonzero b element
    {
        int row = ipvt[i];

        double swap = b[row];
        b[row] = b[i];
        b[i] = swap;
        if( swap != 0 ) break;
    }

    int bi = i++;
    for( /*i = bi*/; i < n; i++ )
    {
        int    row = ipvt[i];
        double tot = b[row];

        b[row] = b[i];
        
        for( int j=bi; j<i; j++ ) tot -= a[i][j]*b[j]; // forward substitution using the lower triangular matrix

        b[i] = tot;
    }
    bool isOk = true;
    
    for( i=n-1; i>=0; i-- )
    {
        double tot = b[i];

        // back-substitution using the upper triangular matrix
        for( int j=i+1; j<n; j++ ) tot -= a[i][j]*b[j];
        
        double volt = tot/a[i][i];
        b[i] = volt;
        
        if( std::isnan( volt ) ) 
        {
            isOk = false;
            volt = 0;
        }
        m_eNodeActive->at(i)->setVolt( volt );      // Set Node Voltages
    }
    return isOk;
}

void CircMatrix::setCircChanged()
{ 
    m_circChanged  = true; 
    m_admitChanged = true;
}

void CircMatrix::printMatrix()
{
    std::cout << "\nAdmitance Matrix:\n"<< std::endl;
    for( int i=0; i<m_numEnodes; i++ )
    {
        for( int j=0; j<m_numEnodes; j++ )
        {
            std::cout << m_circMatrix[i][j] <<"\t";
        }
        std::cout << "\t";
        std::cout << m_coefVect[i]<< std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
    }
    
    //std::cout << "\nSantized Matrix:\n"<< std::endl;
    
}

