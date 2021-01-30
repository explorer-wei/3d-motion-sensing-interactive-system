//=============================================================================
// SkinMesh.cpp: 蒙皮网格模型类的实现
//=============================================================================

#include "dxut.h"
#include "SkinMesh.h"
#include "SDKmisc.h"
#include <string>
using namespace std;

static int BoneId = 0, id = 0;

extern float fRoll, fPitch, fYaw;
extern D3DXMATRIX matRot;
extern string part_control;


//--------------------------------------------------------------------
// Desc: 构造函数和析构函数  Construction/Destruction
//--------------------------------------------------------------------
CSkinMesh::CSkinMesh()
{
	m_bPlayAnim          = true;
	m_pd3dDevice         = NULL;
    m_pAnimController    = NULL;
    m_pFrameRoot         = NULL;
    m_pBoneMatrices      = NULL;
    m_NumBoneMatricesMax = 0;

	m_pAlloc = new CAllocateHierarchy();
}


//-----------------------------------------------------------------------------
// Desc: 构造函数和析构函数 
//-----------------------------------------------------------------------------
CSkinMesh::~CSkinMesh()
{
	D3DXFrameDestroy(m_pFrameRoot, m_pAlloc);
    SAFE_RELEASE(m_pAnimController);
	delete m_pAlloc;
}


//-----------------------------------------------------------------------------
// Desc:创建并加载蒙皮网格模型
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::OnCreate(LPDIRECT3DDEVICE9 pD3DDevice, WCHAR *strFileName)
{
	HRESULT hr;
	m_pd3dDevice = pD3DDevice;
	hr = LoadFromXFile(strFileName);
	if(FAILED(hr))
		return hr;
	return S_OK;
}


//-----------------------------------------------------------------------------
// Desc: 从文件加载蒙皮网格模型
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::LoadFromXFile(WCHAR *strFileName)
{
    HRESULT hr;

	//根据文件名, 从指定的路经查找文件
	WCHAR strPath[MAX_PATH];
	DXUTFindDXSDKMediaFileCch( strPath, sizeof(strPath) / sizeof(WCHAR), strFileName );

	//从.X文件加载层次框架和动画数据
    V_RETURN(D3DXLoadMeshHierarchyFromX(strPath, D3DXMESH_MANAGED, m_pd3dDevice, 
		                            m_pAlloc, NULL, &m_pFrameRoot, &m_pAnimController));
	
	//建立各级框架的组合变换矩阵
    V_RETURN(SetupBoneMatrixPointers(m_pFrameRoot));  
	
	//计算框架对象的边界球
    hr = D3DXFrameCalculateBoundingSphere(m_pFrameRoot, &m_vObjectCenter, &m_fObjectRadius);
	if (FAILED(hr))
        return hr;
	
	return S_OK;
}


//--------------------------------------------------------------------------
// Desc: 仅在LoadFromXFile中调用。调用子函数SetupBoneMatrixPointersOnMesh()
//       安置好各级框架(实际上是各个骨骼)的组合变换矩阵。
// 注意: 在这里其实并没有计算出各个骨骼的组合变换矩阵，只是为每个矩阵开辟了相应
//       的存储空间，真正的计算是在函数CSkinMesh::UpdateFrameMatrices()中完成的。
//---------------------------------------------------------------------------
HRESULT CSkinMesh::SetupBoneMatrixPointers(LPD3DXFRAME pFrame)
{
    HRESULT hr;

    if (pFrame->pMeshContainer != NULL)
    {
        hr = SetupBoneMatrixPointersOnMesh(pFrame->pMeshContainer);  //调用子函数
        if (FAILED(hr))
            return hr;
    }

    if (pFrame->pFrameSibling != NULL)
    {
        hr = SetupBoneMatrixPointers(pFrame->pFrameSibling);   //递归
        if (FAILED(hr))
            return hr;
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
        hr = SetupBoneMatrixPointers(pFrame->pFrameFirstChild);  //递归
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Desc: 仅在SetupBoneMatrixPointers()中被调用，设置每个骨骼的组合变换矩阵
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER pMeshContainerBase)
{
    UINT iBone, cBones;  // cBones表示骨骼数量，iBone表示循环变量
    D3DXFRAME_DERIVED *pFrame;

	//先强制转为扩展型
    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

    // 只有蒙皮网格模型才有骨骼矩阵
    if (pMeshContainer->pSkinInfo != NULL)
    {
		//得到骨骼数量
        cBones = pMeshContainer->pSkinInfo->GetNumBones();

		//申请存储骨骼矩阵的空间
        pMeshContainer->ppBoneMatrixPtrs = new D3DXMATRIX*[cBones];     
		if (pMeshContainer->ppBoneMatrixPtrs == NULL)
            return E_OUTOFMEMORY;

        for (iBone = 0; iBone < cBones; iBone++)
        {
			//找到框架
            pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(m_pFrameRoot, pMeshContainer->pSkinInfo->GetBoneName(iBone));
            if (pFrame == NULL)
                return E_FAIL;

			//将框架的组合变换矩阵赋值给对应的骨骼的复合变换矩阵
            pMeshContainer->ppBoneMatrixPtrs[iBone] = &pFrame->CombinedTransformationMatrix;
		}
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Desc: 更新框架并绘制框架
//       (1)用m_pAnimController->AdvanceTime()设置时间，m_pAnimController是
//          类LPD3DXANIMATIONCONTROLLER的一个对象
//       (2)用函数CSkinMesh::UpdateFrameMatrices()更新框架
//       (3)用函数CSkinMesh::DrawFrame()绘制框架
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::Render(D3DXMATRIXA16* matWorld, float fElapsedAppTime)
{
    if( 0.0f == fElapsedAppTime ) 
        return S_OK;

	BoneId = 0;
	UpdateFrameMatrices(m_pFrameRoot, matWorld );  //调用子函数
	DrawFrame(m_pFrameRoot);  //调用子函数

	return S_OK;
}


//-----------------------------------------------------------------------------
// Desc:计算各个骨骼的组合变换矩阵
//-----------------------------------------------------------------------------
VOID CSkinMesh::UpdateFrameMatrices(LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix)
{
    D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;

	D3DXQUATERNION qR;
	D3DXMATRIX matRot;
	D3DXQuaternionRotationYawPitchRoll( &qR, fYaw, fPitch, fRoll );
	D3DXMatrixRotationQuaternion( &matRot, &qR );

	if ( pFrame->Name != NULL )
	{
		if ( pFrame->Name == part_control)
		{
			D3DXMatrixMultiply(&pFrame->TransformationMatrix, &matRot, &pFrame->TransformationMatrix);
		}
	}

	if (pParentMatrix != NULL)
		D3DXMatrixMultiply(&pFrame->CombinedTransformationMatrix, &pFrame->TransformationMatrix, pParentMatrix);
	else
		pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix;

	if (pFrame->pFrameSibling != NULL)
	{
		UpdateFrameMatrices(pFrame->pFrameSibling, pParentMatrix);
	}

	if (pFrame->pFrameFirstChild != NULL)
	{
		UpdateFrameMatrices(pFrame->pFrameFirstChild, &pFrame->CombinedTransformationMatrix);
	}
}


//-----------------------------------------------------------------------------
// Desc: 绘制框架.
//       先用CSkinMesh::DrawMeshContainer()绘制一个LPD3DXMESHCONTAINER类型
//       的变量pMeshContainer.然后递归绘制同一级框架和子一级框架。
//-----------------------------------------------------------------------------
VOID CSkinMesh::DrawFrame(LPD3DXFRAME pFrame)
{
    LPD3DXMESHCONTAINER pMeshContainer;
	
    pMeshContainer = pFrame->pMeshContainer;
    while (pMeshContainer != NULL)
    {
        DrawMeshContainer(pMeshContainer, pFrame); //调用子函数
        pMeshContainer = pMeshContainer->pNextMeshContainer;
    }
	
    if (pFrame->pFrameSibling != NULL)
    {
        DrawFrame(pFrame->pFrameSibling);
    }
	
    if (pFrame->pFrameFirstChild != NULL)
    {
        DrawFrame(pFrame->pFrameFirstChild);
    }
}


//-----------------------------------------------------------------------------
// Name: DrawMeshContainer()
// Desc: Called to render a mesh in the hierarchy
//-----------------------------------------------------------------------------
VOID CSkinMesh::DrawMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase, LPD3DXFRAME pFrameBase)
{
	D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
	D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;
	UINT iMaterial;
	UINT iAttrib;
	LPD3DXBONECOMBINATION pBoneComb;

	UINT iMatrixIndex;
	UINT iPaletteEntry;
	D3DXMATRIXA16 matTemp;

	if (pMeshContainer->pSkinInfo != NULL) //如果是蒙皮网格
	{
		//检查是否使用软件顶点混合
		HRESULT hr;

		//如果当前硬件不支持, 则使用软件顶点处理
		if (pMeshContainer->UseSoftwareVP)
			V( m_pd3dDevice->SetSoftwareVertexProcessing(true) );

		//启用索引顶点混合
		if (pMeshContainer->NumInfl)
			m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, true);
		
		//根据影响当前网格模型顶点的骨骼数量, 设置需要使用的混合矩阵索引数量
		if (pMeshContainer->NumInfl == 1)
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
		else if(pMeshContainer->NumInfl == 2)
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
		else if(pMeshContainer->NumInfl == 3)
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_2WEIGHTS);
		else
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS);

		//逐个子网格渲染进行渲染
		pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer());
		for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
		{
			//设置混合矩阵
			for (iPaletteEntry = 0; iPaletteEntry < pMeshContainer->NumPaletteEntries; ++iPaletteEntry)
			{
				iMatrixIndex = pBoneComb[iAttrib].BoneId[iPaletteEntry];
				if (iMatrixIndex != UINT_MAX)
				{
					D3DXMatrixMultiply( &matTemp, &pMeshContainer->pBoneOffsetMatrices[iMatrixIndex], 
						                 pMeshContainer->ppBoneMatrixPtrs[iMatrixIndex] );
					m_pd3dDevice->SetTransform( D3DTS_WORLDMATRIX( iPaletteEntry ), &matTemp );
				}
			}

			//设置材质和纹理
			m_pd3dDevice->SetMaterial( &pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D );
			m_pd3dDevice->SetTexture( 0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId] );
			
			//渲染子网格模型
			pMeshContainer->MeshData.pMesh->DrawSubset( iAttrib );
		}

		//恢复顶点混合状态
		m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
		m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);

		//恢复顶点处理模式
		if (pMeshContainer->UseSoftwareVP)
			V( m_pd3dDevice->SetSoftwareVertexProcessing(false));
	} 
	else  // 如果只是普通网格，在添加材质后就绘制它。
	{
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &pFrame->CombinedTransformationMatrix);
		for (iMaterial = 0; iMaterial < pMeshContainer->NumMaterials; iMaterial++)
		{
			m_pd3dDevice->SetMaterial( &pMeshContainer->pMaterials[iMaterial].MatD3D );
			m_pd3dDevice->SetTexture( 0, pMeshContainer->ppTextures[iMaterial] );
			pMeshContainer->MeshData.pMesh->DrawSubset(iMaterial);
		}
	}
}


//-----------------------------------------------------------------------------
// Desc: 释放蒙皮网格模型
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::OnDestory()
{
	delete this;
	return S_OK;
}
