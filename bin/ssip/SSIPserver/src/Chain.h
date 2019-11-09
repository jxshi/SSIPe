#ifndef CHAIN_H
#define CHAIN_H

#include "Residue.h"

typedef enum _Type_Chain
{
  Type_Chain_Protein, 
  Type_Chain_SmallMol, 
  Type_Chain_Nucleotide, 
  Type_Chain_MetalIon, 
  Type_Chain_Water
}Type_Chain;

int ChainTypeConvertFromString(char* typeName, Type_Chain* type);
Type_Chain ChainTypeIdentifiedFromResidueName(char *resiName);

typedef struct _Chain
{
  char name[MAX_LENGTH_CHAIN_NAME+1];
  Type_Chain type;

  int residueNum;
  Residue* residues;
} Chain;

int ChainCreate(Chain* pThis);
int ChainDestroy(Chain* pThis);
int ChainCopy(Chain* pThis, Chain* pOther);
char* ChainGetName(Chain* pThis);
int ChainSetName(Chain* pThis, char* newName);
Type_Chain ChainGetType(Chain* pThis);
int ChainSetType(Chain* pThis, Type_Chain newType);
int ChainGetResidueCount(Chain* pThis);
Residue* ChainGetResidue(Chain* pThis, int index);
int ChainInsertResidue(Chain* pThis, int index, Residue* pNewResi);
int ChainRemoveResidue(Chain* pThis, int index);
int ChainAppendResidue(Chain* pThis, Residue* pNewResi);
int ChainReadCoordinate(Chain* pThis, char* coordinateFile);
int ChainCalcAllAtomXYZ(Chain* pThis, ResiTopoSet* topos);

int ChainShowInPDBFormat(Chain* pThis, int resiIndex, int atomIndex, BOOL showHydrogen, FILE* pFile);

#endif
