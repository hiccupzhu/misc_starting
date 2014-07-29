//-----------------------------------------------------------------------------
// File: D3DUtil.cpp
//
// Desc: Shortcut macros and functions for using DX objects
//
//
// Copyright (c) 1997-1999 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------

#include <math.h>
#include <stdio.h>
#include <tchar.h>
#include "D3DUtil.h"

//-----------------------------------------------------------------------------
// Name: D3DUtil_InitMaterial()
// Desc: Helper function called to build a D3DMATERIAL8 structure
//-----------------------------------------------------------------------------
VOID D3DUtil_InitMaterial( D3DMATERIAL9& mtrl, FLOAT r, FLOAT g, FLOAT b,
                           FLOAT a )
{
    ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );
	mtrl.Diffuse.r = mtrl.Ambient.r = r;
	mtrl.Diffuse.g = mtrl.Ambient.g = g;
	mtrl.Diffuse.b = mtrl.Ambient.b = b;
	mtrl.Diffuse.a = mtrl.Ambient.a = a;
}

//-----------------------------------------------------------------------------
// Name: D3DUtil_SetViewMatrix()
// Desc: Given an eye point, a lookat point, and an up vector, this
//       function builds a 4x4 view matrix.
//-----------------------------------------------------------------------------
HRESULT D3DUtil_SetViewMatrix( D3DMATRIX& mat, D3DXVECTOR3& vFrom,
                               D3DXVECTOR3& vAt, D3DXVECTOR3& vWorldUp )
{
    // Get the z basis vector, which points straight ahead. This is the
    // difference from the eyepoint to the lookat point.
    D3DXVECTOR3 vView = vAt - vFrom;

    FLOAT fLength = D3DXVec3Length( &vView );
    if( fLength < 1e-6f )
        return E_INVALIDARG;

    // Normalize the z basis vector
    vView /= fLength;

    // Get the dot product, and calculate the projection of the z basis
    // vector onto the up vector. The projection is the y basis vector.
    FLOAT fDotProduct = D3DXVec3Dot( &vWorldUp, &vView );

    D3DXVECTOR3 vUp = vWorldUp - fDotProduct * vView;

    // If this vector has near-zero length because the input specified a
    // bogus up vector, let's try a default up vector
    if( 1e-6f > ( fLength = D3DXVec3Length( &vUp ) ) )
    {
        vUp = D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) - vView.y * vView;

        // If we still have near-zero length, resort to a different axis.
        if( 1e-6f > ( fLength = D3DXVec3Length( &vUp ) ) )
        {
            vUp = D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) - vView.z * vView;

            if( 1e-6f > ( fLength = D3DXVec3Length( &vUp ) ) )
                return E_INVALIDARG;
        }
    }

    // Normalize the y basis vector
    vUp /= fLength;

    // The x basis vector is found simply with the cross product of the y
    // and z basis vectors
    D3DXVECTOR3 vRight;
	D3DXVec3Cross(&vRight, &vUp, &vView );

    // Start building the matrix. The first three rows contains the basis
    // vectors used to rotate the view to point at the lookat point
    D3DUtil_SetIdentityMatrix( mat );
    mat._11 = vRight.x;    mat._12 = vUp.x;    mat._13 = vView.x;
    mat._21 = vRight.y;    mat._22 = vUp.y;    mat._23 = vView.y;
    mat._31 = vRight.z;    mat._32 = vUp.z;    mat._33 = vView.z;

    // Do the translation values (rotations are still about the eyepoint)
    mat._41 = - D3DXVec3Dot( &vFrom, &vRight );
    mat._42 = - D3DXVec3Dot( &vFrom, &vUp );
    mat._43 = - D3DXVec3Dot( &vFrom, &vView );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DUtil_SetProjectionMatrix()
// Desc: Sets the passed in 4x4 matrix to a perpsective projection matrix built
//       from the field-of-view (fov, in y), aspect ratio, near plane (D),
//       and far plane (F). Note that the projection matrix is normalized for
//       element [3][4] to be 1.0. This is performed so that W-based range fog
//       will work correctly.
//-----------------------------------------------------------------------------
HRESULT D3DUtil_SetProjectionMatrix( D3DMATRIX& mat, FLOAT fFOV, FLOAT fAspect,
                                     FLOAT fNearPlane, FLOAT fFarPlane )
{
    if( fabs(fFarPlane-fNearPlane) < 0.01f )
        return E_INVALIDARG;
    if( fabs(sin(fFOV/2)) < 0.01f )
        return E_INVALIDARG;

    FLOAT w = fAspect * ( cosf(fFOV/2)/sinf(fFOV/2) );
    FLOAT h =   1.0f  * ( cosf(fFOV/2)/sinf(fFOV/2) );
    FLOAT Q = fFarPlane / ( fFarPlane - fNearPlane );

    ZeroMemory( &mat, sizeof(D3DMATRIX) );
    mat._11 = w;
    mat._22 = h;
    mat._33 = Q;
    mat._34 = 1.0f;
    mat._43 = -Q*fNearPlane;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: D3DUtil_SetRotateXMatrix()
// Desc: Create Rotation matrix about X axis
//-----------------------------------------------------------------------------
VOID D3DUtil_SetRotateXMatrix( D3DMATRIX& mat, FLOAT fRads )
{
    D3DUtil_SetIdentityMatrix( mat );
    mat._22 =  cosf( fRads );
    mat._23 =  sinf( fRads );
    mat._32 = -sinf( fRads );
    mat._33 =  cosf( fRads );
}




//-----------------------------------------------------------------------------
// Name: D3DUtil_SetRotateYMatrix()
// Desc: Create Rotation matrix about Y axis
//-----------------------------------------------------------------------------
VOID D3DUtil_SetRotateYMatrix( D3DMATRIX& mat, FLOAT fRads )
{
    D3DUtil_SetIdentityMatrix( mat );
    mat._11 =  cosf( fRads );
    mat._13 = -sinf( fRads );
    mat._31 =  sinf( fRads );
    mat._33 =  cosf( fRads );
}




//-----------------------------------------------------------------------------
// Name: D3DUtil_SetRotateZMatrix()
// Desc: Create Rotation matrix about Z axis
//-----------------------------------------------------------------------------
VOID D3DUtil_SetRotateZMatrix( D3DMATRIX& mat, FLOAT fRads )
{
    D3DUtil_SetIdentityMatrix( mat );
    mat._11  =  cosf( fRads );
    mat._12  =  sinf( fRads );
    mat._21  = -sinf( fRads );
    mat._22  =  cosf( fRads );
}




//-----------------------------------------------------------------------------
// Name: D3DUtil_SetRotationMatrix
// Desc: Create a Rotation matrix about vector direction
//-----------------------------------------------------------------------------
VOID D3DUtil_SetRotationMatrix( D3DMATRIX& mat, D3DXVECTOR3& vDir, FLOAT fRads )
{
    FLOAT     fCos = cosf( fRads );
    FLOAT     fSin = sinf( fRads );
    D3DXVECTOR3 v;
	D3DXVec3Normalize(&v, &vDir );

    mat._11 = ( v.x * v.x ) * ( 1.0f - fCos ) + fCos;
    mat._12 = ( v.x * v.y ) * ( 1.0f - fCos ) - (v.z * fSin);
    mat._13 = ( v.x * v.z ) * ( 1.0f - fCos ) + (v.y * fSin);

    mat._21 = ( v.y * v.x ) * ( 1.0f - fCos ) + (v.z * fSin);
    mat._22 = ( v.y * v.y ) * ( 1.0f - fCos ) + fCos ;
    mat._23 = ( v.y * v.z ) * ( 1.0f - fCos ) - (v.x * fSin);

    mat._31 = ( v.z * v.x ) * ( 1.0f - fCos ) - (v.y * fSin);
    mat._32 = ( v.z * v.y ) * ( 1.0f - fCos ) + (v.x * fSin);
    mat._33 = ( v.z * v.z ) * ( 1.0f - fCos ) + fCos;

    mat._14 = mat._24 = mat._34 = 0.0f;
    mat._41 = mat._42 = mat._43 = 0.0f;
    mat._44 = 1.0f;
}





