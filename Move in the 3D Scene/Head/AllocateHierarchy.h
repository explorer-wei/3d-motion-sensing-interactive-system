#pragma once

//---------------------------------------------------------------------
// Desc: 继承自DXDXFRAME结构的结构
//-----------------------------------------------------------------------------
struct D3DXFRAME_DERIVED:public D3DXFRAME
{
	D3DXMATRIXA16 CombinedTransformationMatrix;
};

//------------------------------------------------------------------------------
// Desc: 继承自D3DXMESHCONTAINER结构的结构
//-----------------------------------------------------------------------------
struct D3DXMESHCONTAINER_DERIVED:public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9*        ppTextures;  //纹理数组
	LPD3DXMESH                 pOrigMesh;  //原始网格
	DWORD                      NumInfl;    //每个定点最多受多少个骨骼的影响
	DWORD                      NumAttributeGroups;   //属性组数量，即子网格数量
	LPD3DXBUFFER               pBoneCombinationBuf;  //骨骼结合表
	D3DXMATRIX**               ppBoneMatrixPtrs;     //存放骨骼的组合变换矩阵
	D3DXMATRIX*                pBoneOffsetMatrices;  //存放骨骼的初始变换矩阵
	DWORD                      NumPaletteEntries;    //索引列表上限
	bool                       UseSoftwareVP;        //标志是否使用软件定点处理
};

//-----------------------------------------------------------------------------
// Desc: 该类用来从.X文件加载框架层次和网格模型数据
//-----------------------------------------------------------------------------
class CAllocateHierarchy: public ID3DXAllocateHierarchy
{
private:
	HRESULT GenerateSkinnedMesh(D3DXMESHCONTAINER_DERIVED *pMeshContainer); //创建蒙皮动画的Container，主要处理工作
	HRESULT AllocateName( LPCSTR Name, LPSTR *pNewName );            // 命名函数，即把Name赋给pNewName
	void    RemovePathFromFileName(LPSTR fullPath, LPWSTR fileName); // 从文件路径提取文件名

public:
	//重载的虚函数
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
	STDMETHOD(CreateMeshContainer)( THIS_ LPCSTR              Name, 
		                            CONST D3DXMESHDATA*       pMeshData,
		                            CONST D3DXMATERIAL*       pMaterials, 
		                            CONST D3DXEFFECTINSTANCE* pEffectInstances, 
		                            DWORD                     NumMaterials, 
		                            CONST DWORD*             pAdjacency, 
		                            LPD3DXSKININFO            pSkinInfo, 
		                            LPD3DXMESHCONTAINER*     ppNewMeshContainer);    
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
};
