//=============================================================================
// SkinMesh.cpp: ��Ƥ����ģ�����ʵ��
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
// Desc: ���캯������������  Construction/Destruction
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
// Desc: ���캯������������ 
//-----------------------------------------------------------------------------
CSkinMesh::~CSkinMesh()
{
	D3DXFrameDestroy(m_pFrameRoot, m_pAlloc);
    SAFE_RELEASE(m_pAnimController);
	delete m_pAlloc;
}


//-----------------------------------------------------------------------------
// Desc:������������Ƥ����ģ��
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
// Desc: ���ļ�������Ƥ����ģ��
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::LoadFromXFile(WCHAR *strFileName)
{
    HRESULT hr;

	//�����ļ���, ��ָ����·�������ļ�
	WCHAR strPath[MAX_PATH];
	DXUTFindDXSDKMediaFileCch( strPath, sizeof(strPath) / sizeof(WCHAR), strFileName );

	//��.X�ļ����ز�ο�ܺͶ�������
    V_RETURN(D3DXLoadMeshHierarchyFromX(strPath, D3DXMESH_MANAGED, m_pd3dDevice, 
		                            m_pAlloc, NULL, &m_pFrameRoot, &m_pAnimController));
	
	//����������ܵ���ϱ任����
    V_RETURN(SetupBoneMatrixPointers(m_pFrameRoot));  
	
	//�����ܶ���ı߽���
    hr = D3DXFrameCalculateBoundingSphere(m_pFrameRoot, &m_vObjectCenter, &m_fObjectRadius);
	if (FAILED(hr))
        return hr;
	
	return S_OK;
}


//--------------------------------------------------------------------------
// Desc: ����LoadFromXFile�е��á������Ӻ���SetupBoneMatrixPointersOnMesh()
//       ���úø������(ʵ�����Ǹ�������)����ϱ任����
// ע��: ��������ʵ��û�м����������������ϱ任����ֻ��Ϊÿ�����󿪱�����Ӧ
//       �Ĵ洢�ռ䣬�����ļ������ں���CSkinMesh::UpdateFrameMatrices()����ɵġ�
//---------------------------------------------------------------------------
HRESULT CSkinMesh::SetupBoneMatrixPointers(LPD3DXFRAME pFrame)
{
    HRESULT hr;

    if (pFrame->pMeshContainer != NULL)
    {
        hr = SetupBoneMatrixPointersOnMesh(pFrame->pMeshContainer);  //�����Ӻ���
        if (FAILED(hr))
            return hr;
    }

    if (pFrame->pFrameSibling != NULL)
    {
        hr = SetupBoneMatrixPointers(pFrame->pFrameSibling);   //�ݹ�
        if (FAILED(hr))
            return hr;
    }

    if (pFrame->pFrameFirstChild != NULL)
    {
        hr = SetupBoneMatrixPointers(pFrame->pFrameFirstChild);  //�ݹ�
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Desc: ����SetupBoneMatrixPointers()�б����ã�����ÿ����������ϱ任����
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::SetupBoneMatrixPointersOnMesh(LPD3DXMESHCONTAINER pMeshContainerBase)
{
    UINT iBone, cBones;  // cBones��ʾ����������iBone��ʾѭ������
    D3DXFRAME_DERIVED *pFrame;

	//��ǿ��תΪ��չ��
    D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

    // ֻ����Ƥ����ģ�Ͳ��й�������
    if (pMeshContainer->pSkinInfo != NULL)
    {
		//�õ���������
        cBones = pMeshContainer->pSkinInfo->GetNumBones();

		//����洢��������Ŀռ�
        pMeshContainer->ppBoneMatrixPtrs = new D3DXMATRIX*[cBones];     
		if (pMeshContainer->ppBoneMatrixPtrs == NULL)
            return E_OUTOFMEMORY;

        for (iBone = 0; iBone < cBones; iBone++)
        {
			//�ҵ����
            pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind(m_pFrameRoot, pMeshContainer->pSkinInfo->GetBoneName(iBone));
            if (pFrame == NULL)
                return E_FAIL;

			//����ܵ���ϱ任����ֵ����Ӧ�Ĺ����ĸ��ϱ任����
            pMeshContainer->ppBoneMatrixPtrs[iBone] = &pFrame->CombinedTransformationMatrix;
		}
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Desc: ���¿�ܲ����ƿ��
//       (1)��m_pAnimController->AdvanceTime()����ʱ�䣬m_pAnimController��
//          ��LPD3DXANIMATIONCONTROLLER��һ������
//       (2)�ú���CSkinMesh::UpdateFrameMatrices()���¿��
//       (3)�ú���CSkinMesh::DrawFrame()���ƿ��
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::Render(D3DXMATRIXA16* matWorld, float fElapsedAppTime)
{
    if( 0.0f == fElapsedAppTime ) 
        return S_OK;

	BoneId = 0;
	UpdateFrameMatrices(m_pFrameRoot, matWorld );  //�����Ӻ���
	DrawFrame(m_pFrameRoot);  //�����Ӻ���

	return S_OK;
}


//-----------------------------------------------------------------------------
// Desc:���������������ϱ任����
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
// Desc: ���ƿ��.
//       ����CSkinMesh::DrawMeshContainer()����һ��LPD3DXMESHCONTAINER����
//       �ı���pMeshContainer.Ȼ��ݹ����ͬһ����ܺ���һ����ܡ�
//-----------------------------------------------------------------------------
VOID CSkinMesh::DrawFrame(LPD3DXFRAME pFrame)
{
    LPD3DXMESHCONTAINER pMeshContainer;
	
    pMeshContainer = pFrame->pMeshContainer;
    while (pMeshContainer != NULL)
    {
        DrawMeshContainer(pMeshContainer, pFrame); //�����Ӻ���
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

	if (pMeshContainer->pSkinInfo != NULL) //�������Ƥ����
	{
		//����Ƿ�ʹ�����������
		HRESULT hr;

		//�����ǰӲ����֧��, ��ʹ��������㴦��
		if (pMeshContainer->UseSoftwareVP)
			V( m_pd3dDevice->SetSoftwareVertexProcessing(true) );

		//��������������
		if (pMeshContainer->NumInfl)
			m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, true);
		
		//����Ӱ�쵱ǰ����ģ�Ͷ���Ĺ�������, ������Ҫʹ�õĻ�Ͼ�����������
		if (pMeshContainer->NumInfl == 1)
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_0WEIGHTS);
		else if(pMeshContainer->NumInfl == 2)
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
		else if(pMeshContainer->NumInfl == 3)
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_2WEIGHTS);
		else
			m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_3WEIGHTS);

		//�����������Ⱦ������Ⱦ
		pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>(pMeshContainer->pBoneCombinationBuf->GetBufferPointer());
		for (iAttrib = 0; iAttrib < pMeshContainer->NumAttributeGroups; iAttrib++)
		{
			//���û�Ͼ���
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

			//���ò��ʺ�����
			m_pd3dDevice->SetMaterial( &pMeshContainer->pMaterials[pBoneComb[iAttrib].AttribId].MatD3D );
			m_pd3dDevice->SetTexture( 0, pMeshContainer->ppTextures[pBoneComb[iAttrib].AttribId] );
			
			//��Ⱦ������ģ��
			pMeshContainer->MeshData.pMesh->DrawSubset( iAttrib );
		}

		//�ָ�������״̬
		m_pd3dDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
		m_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);

		//�ָ����㴦��ģʽ
		if (pMeshContainer->UseSoftwareVP)
			V( m_pd3dDevice->SetSoftwareVertexProcessing(false));
	} 
	else  // ���ֻ����ͨ��������Ӳ��ʺ�ͻ�������
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
// Desc: �ͷ���Ƥ����ģ��
//-----------------------------------------------------------------------------
HRESULT CSkinMesh::OnDestory()
{
	delete this;
	return S_OK;
}
