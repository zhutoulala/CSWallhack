#pragma once
#include <windows.h>
#include <vector>
#include "d3d9.h"
#include "DXSDK/include/d3dx9.h"
#pragma comment(lib, "d3d9.lib") 
#pragma comment(lib, "d3dx9.lib") 
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "MinHook.lib")
#include "MinHook/include/MinHook.h" //detour

#include <fstream>
using namespace std;
#pragma warning (disable: 4244) //

//==========================================================================================================================

HMODULE dllHandle;

D3DVIEWPORT9 Viewport; //use this viewport
float ScreenCenterX;
float ScreenCenterY;

//Stride
UINT Stride;

//vdesc.Size
//D3DVERTEXBUFFER_DESC vdesc;

//vertexshaderconstantf
UINT mStartRegister;
UINT mVector4fCount;

//get numelements
IDirect3DVertexDeclaration9* pDecl;
D3DVERTEXELEMENT9 decl[MAXD3DDECLLENGTH];
UINT numElements;

// get pshader
//IDirect3DPixelShader9* pShader;
//UINT pSize;

//get vshader
//IDirect3DVertexShader9* vShader;
//UINT vSize;

//texture crc
//IDirect3DTexture9* pCurrentTex = NULL;
//DWORD texCRC;
//int dWidth;
//int dHeight;
//int dFormat;
//int mStage;

//logger
int countnum = -1;

//==========================================================================================================================

//features
int wallhack = 1;
int esp = 1;
int aimbot = 1;
int aimkey = 1;
DWORD Daimkey = VK_SHIFT;		//aimkey
int aimsens = 2;				//aim sensitivity, makes aim smoother
int aimfov = 3;					//aim field of view in % 
								//int aimheight = 2;			//aim height value, not implemented because w2s is bad
int aimheightxy = 1;			//real value, aimheight * 4 + 27
int autoshoot = 1;
unsigned int asdelay = 50;		//use x-999 (shoot for xx millisecs, looks more legit)
bool IsPressd = false;			//

								//timer
DWORD astime = timeGetTime();	//autoshoot
								//==========================================================================================================================

								// getdir & log
char dlldir[320];
char* GetDirectoryFile(const char *filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirectoryFile("log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

void DrawPoint(LPDIRECT3DDEVICE9 Device, int baseX, int baseY, int baseW, int baseH, D3DCOLOR Cor)
{
	D3DRECT BarRect = { baseX, baseY, baseX + baseW, baseY + baseH };
	Device->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, Cor, 0, 0);
}

//==========================================================================================================================

//get distance
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

//Legs worldtoscreen
struct AimLegInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDistance;
};
std::vector<AimLegInfo_t>AimLegInfo;

float HandX, HandY;
float LegX, LegY;
void AddLegAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	D3DXVECTOR3 from, toLeg;
	D3DXMATRIX ViewProj, world, worldA, worldB;

	//D3DVIEWPORT9 Viewport;
	//Device->GetViewport(&Viewport);

	//LEGS AIM
	//worldB for legs 88, 91
	Device->GetVertexShaderConstantF(8, ViewProj, 4);
	Device->GetVertexShaderConstantF(58, worldA, 4); //left
	Device->GetVertexShaderConstantF(88, worldB, 4); //right

	world = (worldA * 0.5f) + (worldB * 0.5f); //center

	float w = 0.0f;
	toLeg[0] = ViewProj[0] * world._14 + ViewProj[1] * world._24 + ViewProj[2] * world._34 + ViewProj[3];
	toLeg[1] = ViewProj[4] * world._14 + ViewProj[5] * world._24 + ViewProj[6] * world._34 + ViewProj[7] + 70.0f; //+height
	w = ViewProj[12] * world._14 + ViewProj[13] * world._24 + ViewProj[14] * world._34 + ViewProj[15];

	if (w > 0.01f)
	{
		float invw = 1.0f / w;
		toLeg[0] *= invw;
		toLeg[1] *= invw;

		float x = Viewport.Width / 2.0f;
		float y = Viewport.Height / 2.0f;

		x += 0.5f * toLeg[0] * Viewport.Width + 0.5f;
		y -= 0.5f * toLeg[1] * Viewport.Height + 0.5f;
		toLeg[0] = x + Viewport.X;
		toLeg[1] = y + Viewport.Y;

		LegX = toLeg[0];
		LegY = toLeg[1];
	}
	else
	{
		toLeg[0] = -1.0f;
		toLeg[1] = -1.0f;
	}

	AimLegInfo_t pAimLegInfo = { static_cast<float>(toLeg[0]), static_cast<float>(toLeg[1]), iTeam };
	AimLegInfo.push_back(pAimLegInfo);
}


//Hands worldtoscreen
struct AimHandInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDistance;
};
std::vector<AimHandInfo_t>AimHandInfo;

void AddHandAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	//float centeredX, centeredY;
	D3DXVECTOR3 from, toHand;
	D3DXMATRIX ViewProj, world, worldA, worldB, worldC;

	//D3DVIEWPORT9 Viewport;
	//Device->GetViewport(&Viewport);

	//HANDS AIM
	//worldC for hands CT only 85, 91, 97, 109, 127, 130, 166, 169, 190, 199, 202, 205	
	//worldC for hands T only 121, 148, 154, 175, 184
	Device->GetVertexShaderConstantF(8, ViewProj, 4);
	Device->GetVertexShaderConstantF(58, worldA, 4); //left
	Device->GetVertexShaderConstantF(160, worldB, 4); //right

	if (iTeam == 1) //CT
		Device->GetVertexShaderConstantF(190, worldC, 4); //move back
	else //T
		Device->GetVertexShaderConstantF(121, worldC, 4); //move back

	world = (worldA * 0.3333f) + (worldB * 0.3333f) + (worldC * 0.3333f); //center

	float w = 0.0f;
	toHand[0] = ViewProj[0] * world._14 + ViewProj[1] * world._24 + ViewProj[2] * world._34 + ViewProj[3];
	toHand[1] = ViewProj[4] * world._14 + ViewProj[5] * world._24 + ViewProj[6] * world._34 + ViewProj[7];
	w = ViewProj[12] * world._14 + ViewProj[13] * world._24 + ViewProj[14] * world._34 + ViewProj[15];

	if (w > 0.01f)
	{
		float invw = 1.0f / w;
		toHand[0] *= invw;
		toHand[1] *= invw;

		float x = Viewport.Width / 2.0f;
		float y = Viewport.Height / 2.0f;

		x += 0.5f * toHand[0] * Viewport.Width + 0.5f;
		y -= 0.5f * toHand[1] * Viewport.Height + 0.5f;
		toHand[0] = x + Viewport.X;
		toHand[1] = y + Viewport.Y;

		HandX = toHand[0];
		HandY = toHand[1];
	}
	else
	{
		toHand[0] = -1.0f;
		toHand[1] = -1.0f;
	}

	AimHandInfo_t pAimHandInfo = { static_cast<float>(toHand[0]), static_cast<float>(toHand[1]), iTeam };
	AimHandInfo.push_back(pAimHandInfo);
}

//Centered worldtoscreen
struct AimCenteredInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDistance;
};
std::vector<AimCenteredInfo_t>AimCenteredInfo;

void AddCenteredAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	float centeredX, centeredY;
	if (LegX > 0 && HandX > 0)
	{
		centeredX = (HandX + LegX) / 2.0f;
		centeredY = (HandY + LegY) / 2.0f;
		//DrawPoint(Device, (int)centeredX, (int)centeredY, 10, 10, 0xFFFFFF00);
	}

	AimCenteredInfo_t pAimCenteredInfo = { static_cast<float>(centeredX), static_cast<float>(centeredY), iTeam };
	AimCenteredInfo.push_back(pAimCenteredInfo);
}



/*
//test worldtoscreen
struct AimInfo_t
{
float vOutX, vOutY;
INT       iTeam;
float CrosshairDistance;
};
std::vector<AimInfo_t>AimInfo;
void AddAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
D3DXVECTOR3 from, to;
D3DXMATRIX ViewProj, world, worldA, worldB, worldC;
//D3DVIEWPORT9 Viewport;
//Device->GetViewport(&Viewport);
//Hands AIM
//Device->GetVertexShaderConstantF(8, ViewProj, 4);
//Device->GetVertexShaderConstantF(58, worldA, 4);
//Device->GetVertexShaderConstantF(160, worldB, 4); //right
//Device->GetVertexShaderConstantF(190, worldC, 4); //move back
//world = (worldA * 0.3333f) + (worldB * 0.3333f) + (worldC * 0.3333f); //center
//LEGS AIM
//worldB for legs 88, 91
//Device->GetVertexShaderConstantF(8, ViewProj, 4);
//Device->GetVertexShaderConstantF(58, worldA, 4); //left
//Device->GetVertexShaderConstantF(88, worldB, 4); //right
world = (worldA * 0.5f) + (worldB * 0.5f); //center
float w = 0.0f;
to[0] = ViewProj[0] * world._14 + ViewProj[1] * world._24 + ViewProj[2] * world._34 + ViewProj[3];
to[1] = ViewProj[4] * world._14 + ViewProj[5] * world._24 + ViewProj[6] * world._34 + ViewProj[7]; //+height
w = ViewProj[12] * world._14 + ViewProj[13] * world._24 + ViewProj[14] * world._34 + ViewProj[15];
if (w > 0.01f)
{
float invw = 1.0f / w;
to[0] *= invw;
to[1] *= invw;
float x = Viewport.Width / 2.0f;
float y = Viewport.Height / 2.0f;
x += 0.5f * to[0] * Viewport.Width + 0.5f;
y -= 0.5f * to[1] * Viewport.Height + 0.5f;
to[0] = x + Viewport.X;
to[1] = y + Viewport.Y;
}
else
{
to[0] = -1.0f;
to[1] = -1.0f;
}
AimInfo_t pAimInfo = { static_cast<float>(to[0]), static_cast<float>(to[1]), iTeam };
AimInfo.push_back(pAimInfo);
}
*/


//==========================================================================================================================

//model rec (bodies are wrong/outdated)

//magic
#define CT_HANDS (Stride == 32 && decl->Type == 2 && numElements >= 10 && mStartRegister == 58 && mVector4fCount == 150)// && mStage == 0)
#define T_HANDS ((Stride == 32 && decl->Type == 2 && numElements >= 10 && mStartRegister == 58)&&(mVector4fCount == 108||mVector4fCount == 144||mVector4fCount == 126))// && mStage == 0)

//////////////////////
//		CT 			//
//////////////////////


//IDF
#define ct_idf_legs	((Stride==32) && (NumVertices==2118) && (primCount==3354))
#define ct_idf_head1 ((Stride==32) && (NumVertices==1761) && (primCount==2681))
#define ct_idf_head2 ((Stride==32) && (NumVertices==1677) && (primCount==2713))
#define ct_idf_body ((Stride==32) && (NumVertices==4523) && (primCount==6700))

#define ct_idf ct_idf_legs || ct_idf_head1 || ct_idf_head2 || ct_idf_body


//SAS
#define ct_sas_legs ((Stride==32) && (NumVertices==4442) && (primCount==5475))
#define ct_sas_head ((Stride==32) && (NumVertices==1215) && (primCount==1749))
#define ct_sas_body ((Stride==32) && (NumVertices==5443) && (primCount==8008))

//New d3d model rec for SAS(12 / 13 / 2016 csgo update) :
#define new_sas_head ((Stride==32) && (NumVertices == 5299) && (primCount == 7167))//head -
#define new_sas_eyes ((Stride==32) && (NumVertices == 136) && (primCount == 204))//eyes - 
#define new_sas_chest ((Stride==32) && (NumVertices == 1967) && (primCount == 2720))//chest - 
#define new_sas_hands ((Stride==32) && (NumVertices == 2052) && (primCount == 2966))//hands - 
#define new_sas_legs ((Stride==32) && (NumVertices == 2482) && (primCount == 3414))//legs - 

#define ct_sas ct_sas_legs || ct_sas_head || ct_sas_body || new_sas_head || new_sas_eyes || new_sas_chest || new_sas_legs


//FBI
#define ct_fbi_legs ((Stride==32) && (NumVertices==1646) && (primCount==2714))
#define ct_fbi_body ((Stride==32) && (NumVertices==5395) && (primCount==8657))
#define ct_fbi_head1 ((Stride==32) && (NumVertices==1459) && (primCount==2566))
#define ct_fbi_head2 ((Stride==32) && (NumVertices==2223) && (primCount==3672))
#define ct_fbi_head3 ((Stride==32) && (NumVertices==2778) && (primCount==4244))
#define ct_fbi_head4 ((Stride==32) && (NumVertices==993) && (primCount==1722))
#define ct_fbi_head5 ((Stride==32) && (NumVertices==2957) && (primCount==4234))

#define ct_fbi ct_fbi_legs || ct_fbi_body || ct_fbi_head1 || ct_fbi_head2 || ct_fbi_head3 || ct_fbi_head4 || ct_fbi_head5


//SWAT
#define ct_swat_legs ((Stride==32) && (NumVertices==3650) && (primCount==4030))
#define ct_swat_body1 ((Stride==32) && (NumVertices==6016) && (primCount==8022))
#define ct_swat_body2 ((Stride==32) && (NumVertices==5675) && (primCount==7713))
#define ct_swat_body3 ((Stride==32) && (NumVertices==6112) && (primCount==8261))
#define ct_swat_body4 ((Stride==32) && (NumVertices==6777) && (primCount==8930))
#define ct_swat_body5 ((Stride==32) && (NumVertices==6087) && (primCount==8226))
#define ct_swat_head1 ((Stride==32) && (NumVertices==1883) && (primCount==2986))
#define ct_swat_head2 ((Stride==32) && (NumVertices==1917) && (primCount==3004))
#define ct_swat_head3 ((Stride==32) && (NumVertices==1835) && (primCount==2898))
#define ct_swat_head4 ((Stride==32) && (NumVertices==1980) && (primCount==3074))
#define ct_swat_head5 ((Stride==32) && (NumVertices==1834) && (primCount==2898))

#define ct_swat ct_swat_legs || ct_swat_body1 || ct_swat_body2 || ct_swat_body3 || ct_swat_body4 || ct_swat_body5 || ct_swat_head1 || ct_swat_head2 || ct_swat_head3 || ct_swat_head4 || ct_swat_head5


//GSG9
#define ct_gsg9_legs ((Stride==32) && (NumVertices==1410) && (primCount==2352))
#define ct_gsg9_body ((Stride==32) && (NumVertices==3301) && (primCount==5424))
#define ct_gsg9_head1 ((Stride==32) && (NumVertices==2113) && (primCount==3391))
#define ct_gsg9_head2 ((Stride==32) && (NumVertices==2157) && (primCount==3461))
#define ct_gsg9_head3 ((Stride==32) && (NumVertices==2151) && (primCount==3459))

#define ct_gsg9 ct_gsg9_legs || ct_gsg9_body || ct_gsg9_head1 || ct_gsg9_head2 || ct_gsg9_head3


//GIGN
#define ct_gign_legs ((Stride==32) && (NumVertices==1258) && (primCount==1950))
#define ct_gign_body ((Stride==32) && (NumVertices==4854) && (primCount==7410))
#define ct_gign_head1 ((Stride==32) && (NumVertices==1645) && (primCount==2568))
#define ct_gign_head2 ((Stride==32) && (NumVertices==1611) && (primCount==2522))
#define ct_gign_head3 ((Stride==32) && (NumVertices==1601) && (primCount==2502))

#define ct_gign ct_gign_legs || ct_gign_body || ct_gign_head1 || ct_gign_head2 || ct_gign_head3


//SEAL
#define ct_seal_legs1 ((Stride==32) && (NumVertices==2924) && (primCount==3540))
#define ct_seal_legs2 ((Stride==32) && (NumVertices==2923) && (primCount==3540))
#define ct_seal_body1 ((Stride==32) && (NumVertices==5547) && (primCount==8003))
#define ct_seal_body2 ((Stride==32) && (NumVertices==5524) && (primCount==8106))
#define ct_seal_body3 ((Stride==32) && (NumVertices==5680) && (primCount==8166))
#define ct_seal_body4 ((Stride==32) && (NumVertices==5334) && (primCount==7910))
#define ct_seal_body5 ((Stride==32) && (NumVertices==5436) && (primCount==8003))
#define ct_seal_head1 ((Stride==32) && (NumVertices==2539) && (primCount==3527))
#define ct_seal_head2 ((Stride==32) && (NumVertices==3137) && (primCount==3948))
#define ct_seal_head3 ((Stride==32) && (NumVertices==3245) && (primCount==4158))
#define ct_seal_head4 ((Stride==32) && (NumVertices==2963) && (primCount==3792))
#define ct_seal_head5 ((Stride==32) && (NumVertices==4084) && (primCount==4697))

#define ct_seal ct_seal_legs1 || ct_seal_legs2 || ct_seal_body1 || ct_seal_body2 || ct_seal_body3 || ct_seal_body4 || ct_seal_body5 || ct_seal_head1 || ct_seal_head2 || ct_seal_head3 || ct_seal_head4 || ct_seal_head5

//CT Models
#define CT_Models (ct_idf || ct_sas || ct_fbi || ct_swat || ct_gsg9 || ct_gign || ct_seal)

//CT Heads
#define CT_Heads (ct_idf_head1 || ct_idf_head2 || ct_sas_head || ct_fbi_head1 || ct_fbi_head2 || ct_fbi_head3 || ct_fbi_head4 || ct_fbi_head5 || ct_swat_head1 || ct_swat_head2 || ct_swat_head3 || ct_swat_head4 || ct_swat_head5 || ct_gsg9_head1 || ct_gsg9_head2 || ct_gsg9_head3 || ct_gign_head1 || ct_gign_head2 || ct_gign_head3 || ct_seal_head1 || ct_seal_head2 || ct_seal_head3 || ct_seal_head4 || ct_seal_head5)

//CT Legs
#define CT_LEGS (ct_idf_legs||ct_sas_legs||new_sas_legs||ct_fbi_legs||ct_swat_legs||ct_gsg9_legs||ct_gign_legs||ct_seal_legs1||ct_seal_legs2) 

//////////////////////
//		T 			//
//////////////////////


//ELITE CREW
#define t_elite1_legs ((Stride==32) && (NumVertices==2531) && (primCount==3888))
#define t_elite1_head ((Stride==32) && (NumVertices==1134) && (primCount==2024))
#define t_elite1_body ((Stride==32) && (NumVertices==3606) && (primCount==5856))
#define t_elite2_legs ((Stride==32) && (NumVertices==1889) && (primCount==3088))
#define t_elite2_head ((Stride==32) && (NumVertices==1310) && (primCount==2302))
#define t_elite2_body ((Stride==32) && (NumVertices==3816) && (primCount==5930))
#define t_elite3_legs ((Stride==32) && (NumVertices==2416) && (primCount==3778))
#define t_elite3_head ((Stride==32) && (NumVertices==1430) && (primCount==2422))
#define t_elite3_body ((Stride==32) && (NumVertices==2587) && (primCount==4334))
#define t_elite4_legs ((Stride==32) && (NumVertices==2138) && (primCount==3424))
#define t_elite4_head ((Stride==32) && (NumVertices==1352) && (primCount==2268))
#define t_elite5_legs ((Stride==32) && (NumVertices==1986) && (primCount==3044))
#define t_elite5_head ((Stride==32) && (NumVertices==1372) && (primCount==2286))
#define t_elite5_body ((Stride==32) && (NumVertices== 3162) && (primCount==5182))


#define t_elite t_elite1_legs || t_elite1_head || t_elite1_body || t_elite2_legs || t_elite2_head || t_elite2_body || t_elite3_legs || t_elite3_head || t_elite3_body || t_elite4_legs || t_elite4_head || t_elite5_legs || t_elite5_head || t_elite5_body


//Separatist
#define t_separatist_legs ((Stride==32) && (NumVertices==2536) && (primCount==4196))
#define t_separatist_head ((Stride==32) && (NumVertices==1106) && (primCount==2018))
#define t_separatist_body ((Stride==32) && (NumVertices==3210) && (primCount==5382))

#define t_separatist t_separatist_legs || t_separatist_head || t_separatist_body


//Anarchist
#define t_anarchist_legs ((Stride==32) && (NumVertices==1671) && (primCount==2876))
#define t_anarchist_head ((Stride==32) && (NumVertices==1925) && (primCount==3314))
#define t_anarchist_body ((Stride==32) && (NumVertices==3881) && (primCount==6774))

#define t_anarchist t_anarchist_legs || t_anarchist_head || t_anarchist_body


//Professional
#define t_professional_legs ((Stride==32) && (NumVertices==2056) && (primCount==3436))
#define t_professional_body ((Stride==32) && (NumVertices==5116) && (primCount==8253))
#define t_professional_head1 ((Stride==32) && (NumVertices==927) && (primCount==1626))
#define t_professional_head2 ((Stride==32) && (NumVertices==646) && (primCount==1248))
#define t_professional_head3 ((Stride==32) && (NumVertices==1053) && (primCount==1968))
#define t_professional_head4 ((Stride==32) && (NumVertices==1197) && (primCount==2256))
#define t_professional_head5 ((Stride==32) && (NumVertices==924) && (primCount==1632))
#define t_professional_glasses1 ((Stride==32) && (NumVertices==266) && (primCount==348))
#define t_professional_glasses2 ((Stride==32) && (NumVertices==200) && (primCount==276))

#define t_professional t_professional_legs || t_professional_body || t_professional_head1 || t_professional_head2 || t_professional_head3 || t_professional_head4 || t_professional_head5 || t_professional_glasses1 || t_professional_glasses2


//Phoenix
#define t_phoenix_legs ((Stride==32) && (NumVertices==2599) && (primCount==4097))
#define t_phoenix_body ((Stride==32) && (NumVertices==2944) && (primCount==4916))
#define t_phoenix_head ((Stride==32) && (NumVertices==876) && (primCount==1630))

#define t_phoenix t_phoenix_legs || t_phoenix_body || t_phoenix_head


//Balkan
#define t_balkan_legs1 ((Stride==32) && (NumVertices==1525) && (primCount==2525))
#define t_balkan_legs2 ((Stride==32) && (NumVertices==1273) && (primCount==2113))
#define t_balkan_body ((Stride==32) && (NumVertices==5215) && (primCount==8130))
#define t_balkan_head1 ((Stride==32) && (NumVertices==1214) && (primCount==2292))
#define t_balkan_head2 ((Stride==32) && (NumVertices==1215) && (primCount==2286))
#define t_balkan_head3 ((Stride==32) && (NumVertices==1291) && (primCount==2437))
#define t_balkan_head4 ((Stride==32) && (NumVertices==1345) && (primCount==2488))
#define t_balkan_head5 ((Stride==32) && (NumVertices==1624) && (primCount==2936))

#define t_balkan t_balkan_legs1 || t_balkan_legs2 || t_balkan_body || t_balkan_head1 || t_balkan_head2 || t_balkan_head3 || t_balkan_head4 || t_balkan_head5


//Pirate
#define t_pirate_legs1 ((Stride==32) && (NumVertices==934) && (primCount==1525))
#define t_pirate_legs2 ((Stride==32) && (NumVertices==991) && (primCount==1525))
#define t_pirate_body ((Stride==32) && (NumVertices==7900) && (primCount==9248))
#define t_pirate_head1 ((Stride==32) && (NumVertices==1996) && (primCount==2906))
#define t_pirate_head2 ((Stride==32) && (NumVertices==1183) && (primCount==2045))
#define t_pirate_head3 ((Stride==32) && (NumVertices==1380) && (primCount==2493))
#define t_pirate_head4 ((Stride==32) && (NumVertices==1314) && (primCount==2367))

#define t_pirate t_pirate_legs1 || t_pirate_legs2 || t_pirate_body || t_pirate_head1 || t_pirate_head2 || t_pirate_head3 || t_pirate_head4

//T Models
#define T_Models (t_elite || t_separatist || t_anarchist || t_professional || t_phoenix || t_balkan || t_pirate)

//T Heads
#define T_Heads (t_elite1_head || t_elite2_head|| t_elite3_head||t_elite4_head||t_elite5_head || t_separatist_head || t_anarchist_head || t_professional_head1 || t_professional_head2 || t_professional_head3 || t_professional_head4 || t_professional_head5 || t_phoenix_head || t_balkan_head1 || t_balkan_head2 || t_balkan_head3 || t_balkan_head4 || t_balkan_head5 || t_pirate_head1 || t_pirate_head2 || t_pirate_head3 || t_pirate_head4)

//T Legs
#define T_LEGS (t_elite1_legs||t_elite2_legs||t_elite3_legs||t_elite4_legs||t_elite5_legs||t_separatist_legs||t_anarchist_legs||t_professional_legs||t_phoenix_legs||t_balkan_legs1||t_balkan_legs2||t_pirate_legs1||t_pirate_legs2)


//==========================================================================================================================

//-----------------------------------------------------------------------------
// Name: Save()
// Desc: Saves mensa Item states for later Restoration
//-----------------------------------------------------------------------------
/*
void Save(char* szSection, char* szKey, int iValue, LPCSTR file)
{
char szValue[255];
sprintf_s(szValue, "%d", iValue);
WritePrivateProfileString(szSection, szKey, szValue, file);
}
*/
//-----------------------------------------------------------------------------
// Name: Load()
// Desc: Loads mensa Item States From Previously Saved File
//-----------------------------------------------------------------------------
/*
int Load(char* szSection, char* szKey, int iDefaultValue, LPCSTR file)
{
int iResult = GetPrivateProfileInt(szSection, szKey, iDefaultValue, file);
return iResult;
}
*/

#include <string>
#include <fstream>
void SaveCfg()
{
	ofstream fout;
	fout.open(GetDirectoryFile("csgocfg.ini"), ios::trunc);
	fout << "Wallhack " << wallhack << endl;
	fout << "Esp " << esp << endl;
	fout << "Aimbot " << aimbot << endl;
	fout << "Aimkey " << aimkey << endl;
	fout << "Aimsens " << aimsens << endl;
	fout << "Aimfov " << aimfov << endl;
	//fout << "Aimheight " << aimheight << endl;
	fout << "Autoshoot " << autoshoot << endl;
	fout.close();
}

void LoadCfg()
{
	ifstream fin;
	string Word = "";
	fin.open(GetDirectoryFile("csgocfg.ini"), ifstream::in);
	fin >> Word >> wallhack;
	fin >> Word >> esp;
	fin >> Word >> aimbot;
	fin >> Word >> aimkey;
	fin >> Word >> aimsens;
	fin >> Word >> aimfov;
	//fin >> Word >> aimheight;
	fin >> Word >> autoshoot;
	fin.close();
}

//==========================================================================================================================
/*
// colors
#define Green				D3DCOLOR_ARGB(255, 000, 255, 000)
#define Red					D3DCOLOR_ARGB(255, 255, 000, 000)
#define Blue				D3DCOLOR_ARGB(255, 000, 000, 255)
#define Orange				D3DCOLOR_ARGB(255, 255, 165, 000)
#define Yellow				D3DCOLOR_ARGB(255, 255, 255, 000)
#define Pink				D3DCOLOR_ARGB(255, 255, 192, 203)
#define Cyan				D3DCOLOR_ARGB(255, 000, 255, 255)
#define Purple				D3DCOLOR_ARGB(255, 160, 032, 240)
#define Black				D3DCOLOR_ARGB(255, 000, 000, 000)
#define White				D3DCOLOR_ARGB(255, 255, 255, 255)
#define Grey				D3DCOLOR_ARGB(255, 112, 112, 112)
#define SteelBlue			D3DCOLOR_ARGB(255, 033, 104, 140)
#define LightSteelBlue		D3DCOLOR_ARGB(255, 201, 255, 255)
#define LightBlue			D3DCOLOR_ARGB(255, 026, 140, 306)
#define Salmon				D3DCOLOR_ARGB(255, 196, 112, 112)
#define Brown				D3DCOLOR_ARGB(255, 168, 099, 020)
#define Teal				D3DCOLOR_ARGB(255, 038, 140, 140)
#define Lime				D3DCOLOR_ARGB(255, 050, 205, 050)
#define ElectricLime		D3DCOLOR_ARGB(255, 204, 255, 000)
#define Gold				D3DCOLOR_ARGB(255, 255, 215, 000)
#define OrangeRed			D3DCOLOR_ARGB(255, 255, 69, 0)
#define GreenYellow			D3DCOLOR_ARGB(255, 173, 255, 047)
#define AquaMarine			D3DCOLOR_ARGB(255, 127, 255, 212)
#define SkyBlue				D3DCOLOR_ARGB(255, 000, 191, 255)
#define SlateBlue			D3DCOLOR_ARGB(255, 132, 112, 255)
#define Crimson				D3DCOLOR_ARGB(255, 220, 020, 060)
#define DarkOliveGreen		D3DCOLOR_ARGB(255, 188, 238, 104)
#define PaleGreen			D3DCOLOR_ARGB(255, 154, 255, 154)
#define DarkGoldenRod		D3DCOLOR_ARGB(255, 255, 185, 015)
#define FireBrick			D3DCOLOR_ARGB(255, 255, 048, 048)
#define DarkBlue			D3DCOLOR_ARGB(255, 000, 000, 204)
#define DarkerBlue			D3DCOLOR_ARGB(255, 000, 000, 153)
#define DarkYellow			D3DCOLOR_ARGB(255, 255, 204, 000)
#define LightYellow			D3DCOLOR_ARGB(255, 255, 255, 153)
#define DarkOutline			D3DCOLOR_ARGB(255, 37,   48,  52)
#define TBlack				D3DCOLOR_ARGB(180, 000, 000, 000)
*/

#define White				D3DCOLOR_ARGB(255, 255, 255, 255)
#define Yellow				D3DCOLOR_ARGB(255, 255, 255, 0)
#define TBlack				D3DCOLOR_ARGB(180, 0, 0, 0) 
#define Black				D3DCOLOR_ARGB(255, 0, 0, 0) 
#define Red					D3DCOLOR_ARGB(255, 255, 0, 0)
#define Green				D3DCOLOR_ARGB(255, 0, 255, 0)
#define DarkOutline			D3DCOLOR_ARGB(255, 37, 48, 52)

// mensa

int mensaSelect = 0;
int Current = true;

int PosX = 30;
int PosY = 27;

int Show = false; //off by default

POINT cPos;

#define ItemColorOn Green
#define ItemColorOff Red
#define ItemCurrent White
#define GroupColor Yellow
#define KategorieFarbe Yellow
#define ItemText White

LPD3DXFONT pFont; //font

int CheckTab(int x, int y, int w, int h)
{
	if (Show)
	{
		GetCursorPos(&cPos);
		ScreenToClient(GetForegroundWindow(), &cPos);
		if (cPos.x > x && cPos.x < x + w && cPos.y > y && cPos.y < y + h)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 1)
			{
				//return 1; //disabled mouse selection in menu
			}
			return 2;
		}
	}
	return 0;
}

void FillRGB(LPDIRECT3DDEVICE9 pDevice, int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT rec = { x, y, x + w, y + h };
	pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT DrawRectangle(LPDIRECT3DDEVICE9 Device, FLOAT x, FLOAT y, FLOAT w, FLOAT h, DWORD Color)
{
	HRESULT hRet;

	const DWORD D3D_FVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	struct Vertex
	{
		float x, y, z, ht;
		DWORD vcolor;
	}
	V[4] =
	{
		{ x, (y + h), 0.0f, 0.0f, Color },
	{ x, y, 0.0f, 0.0f, Color },
	{ (x + w), (y + h), 0.0f, 0.0f, Color },
	{ (x + w), y, 0.0f, 0.0f, Color }
	};

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Device->SetPixelShader(0); //fix black color
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetFVF(D3D_FVF);
		Device->SetTexture(0, NULL);
		hRet = Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
	}

	return hRet;
}

VOID DrawBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, INT px, DWORD BorderColor)
{
	DrawRectangle(Device, x, (y + h - px), w, px, BorderColor);
	DrawRectangle(Device, x, y, px, h, BorderColor);
	DrawRectangle(Device, x, y, w, px, BorderColor);
	DrawRectangle(Device, (x + w - px), y, px, h, BorderColor);
}

VOID DrawBoxWithBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor, DWORD BorderColor)
{
	DrawRectangle(Device, x, y, w, h, BoxColor);
	DrawBorder(Device, x, y, w, h, 1, BorderColor);
}

VOID DrawBox(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor)
{
	DrawBorder(Device, x, y, w, h, 1, BoxColor);
}

void WriteText(int x, int y, DWORD color, const char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawTextA(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteText(int x, int y, DWORD color, const char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawTextA(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void cWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawTextA(0, text, -1, &rect, DT_NOCLIP | DT_CENTER, color);
}

HRESULT DrawString(LPD3DXFONT pFont, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 50, 50);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		pFont->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = pFont->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

void Category(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (Show)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = KategorieFarbe;

		if (Check == 2)
			ColorText = ItemCurrent;

		if (mensaSelect == Current)
			ColorText = ItemCurrent;

		WriteText(PosX + 44, PosY + 50 + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 236, PosY + 50 + (Current * 15) - 1, ColorText, "[-]");
		Current++;
	}
}

void AddItem(LPDIRECT3DDEVICE9 pDevice, const char *text, int &var, const char **opt, int MaxValue)
{
	if (Show)
	{
		int Check = CheckTab(PosX + 44, (PosY + 51) + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			//DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 10, 10, Green);
			ColorText = ItemColorOn;
		}
		if (var == 0)
		{
			//DrawBox(pDevice, PosX+44, PosY+51 + (Current * 15), 10, 10, Red);
			ColorText = ItemColorOff;
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = ItemCurrent;

		if (mensaSelect == Current)
		{
			if (GetAsyncKeyState(VK_RIGHT) & 1)
			{
				var++;
				if (var > MaxValue)
					var = 0;
			}
			else if (GetAsyncKeyState(VK_LEFT) & 1)
			{
				var--;
				if (var < 0)
					var = MaxValue;
			}
		}

		if (mensaSelect == Current)
			ColorText = ItemCurrent;


		WriteText(PosX + 43, PosY + 49 + (Current * 15) - 1, Black, text);
		WriteText(PosX + 45, PosY + 51 + (Current * 15) - 1, ColorText, text);

		lWriteText(PosX + 235, PosY + 49 + (Current * 15) - 1, Black, opt[var]);
		lWriteText(PosX + 237, PosY + 51 + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}

//==========================================================================================================================

// menu part
const char *opt_OnOff[] = { "[OFF]", "[On]" };
const char *opt_Teams[] = { "[OFF]", "[CT]", "[T]" };
const char *opt_WhChams[] = { "[OFF]", "[On]", "[On + Chams]" };
const char *opt_Keys[] = { "[OFF]", "[Shift]", "[RMouse]", "[LMouse]", "[Ctrl]", "[Alt]", "[Space]", "[X]", "[C]" };
const char *opt_Sensitivity[] = { "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]", "[11]", "[12]", "[13]", "[14]", "[15]", "[16]", "[17]", "[18]", "[19]", "[20]" };
const char *opt_Aimheight[] = { "[0]", "[1]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]", "[10]" };
const char *opt_Aimfov[] = { "[0]", "[10%]", "[20%]", "[30%]", "[40%]", "[50%]", "[60%]", "[70%]", "[80%]", "[90%]" };
const char *opt_Aimpause[] = { "[0]", "[0.2sec]", "[0.4sec]", "[0.6sec]", "[0.8sec]", "[1.0sec]", "[1.2sec]", "[1.4sec]", "[1.6sec]", "[1.8sec]", "[2.0sec]" };
const char *opt_autoshoot[] = { "[OFF]", "[OnKeyDown]" };

void Drawmenu(LPDIRECT3DDEVICE9 pDevice)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		Show = !Show;

		//save settings
		SaveCfg();

		//Save("wallhack", "wallhack", wallhack, GetFolderFile("palaconfig.ini"));

		//PlaySoundA(GetDirectoryFile("stuff\\sounds\\menu.wav"), 0, SND_FILENAME | SND_ASYNC | SND_NOSTOP | SND_NODEFAULT);
	}

	if (Show && pFont)
	{
		if (GetAsyncKeyState(VK_UP) & 1)
			mensaSelect--;

		if (GetAsyncKeyState(VK_DOWN) & 1)
			mensaSelect++;

		//draw background
		FillRGB(pDevice, 71, 86, 200, 120, TBlack);
		DrawBox(pDevice, 71, 86, 200, Current * 15, DarkOutline);
		//draw mensa pic
		//PrePresent(pDevice, 20, 20);
		//cWriteTex(172, 71, White, "CSGO D3D");

		Current = 1;
		//Categor(pDevice, " [D3D]");
		AddItem(pDevice, " Wallhack", wallhack, opt_WhChams, 1);
		AddItem(pDevice, " Esp", esp, opt_OnOff, 1);
		AddItem(pDevice, " Aimbot", aimbot, opt_Teams, 2);
		AddItem(pDevice, " Aimkey", aimkey, opt_Keys, 8);
		AddItem(pDevice, " Aimsens", aimsens, opt_Sensitivity, 19);
		AddItem(pDevice, " Aimfov", aimfov, opt_Aimfov, 9);
		//AddItem(pDevice, " Aimheight", aimheight, opt_Aimheight, 5);
		AddItem(pDevice, " Autoshoot", autoshoot, opt_autoshoot, 1);

		if (mensaSelect >= Current)
			mensaSelect = 1;

		if (mensaSelect < 1)
			mensaSelect = 7;//Current;
	}
}
