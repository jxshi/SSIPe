///////////////////////////////////////////////////////////////////////////////////////
//Copyright (C) 2017-2019 Yang Zhang Lab, University of Michigan - All Rights Reserved
//Unauthorized copying of this file, via any medium is strictly prohibited
//Written by Xiaoqiang Huang <xiaoqiah@umich.edu>,2017 Winter
//////////////////////////////////////////////////////////////////////////////////////

#include "AtomParamsSet.h"
#include <string.h>
#include "EEF1Parameter.h"
#include "FOLDEFParameter.h"


int AtomParamsSetCreate(AtomParamsSet* pThis){
  StringArrayCreate(&pThis->residueNames);
  pThis->atomCount = NULL;
  pThis->atoms = NULL;
  return Success;
}

int AtomParamsSetDestroy(AtomParamsSet* pThis){
  for(int i=0;i<AtomParamsSetGetResidueCount(pThis);i++){
    for(int j=0;j<pThis->atomCount[i];j++){
      AtomDestroy(&pThis->atoms[i][j]);
    }
    free(pThis->atoms[i]);
  }
  free(pThis->atoms);
  free(pThis->atomCount);
  StringArrayDestroy(&pThis->residueNames);
  return Success;
}

int AtomParamsSetAdd(AtomParamsSet* pThis, char* residueName, Atom* pNewAtom){
  int resiIndex;
  int atomIndex;

  if(strlen(residueName)>MAX_LENGTH_RESIDUE_NAME){
    char usrMsg[MAX_LENGTH_ERR_MSG+1];
    int errorCode = ValueError;
    sprintf(usrMsg, "in file %s function %s() line %d, residue name is too long", __FILE__, __FUNCTION__, __LINE__);
    TraceError(usrMsg, errorCode);
    return errorCode;
  }

  int result = StringArrayFind(&pThis->residueNames, residueName, &resiIndex);
  // new residue
  if(FAILED(result)){
    int newResidueCount = AtomParamsSetGetResidueCount(pThis) + 1;
    pThis->atoms = (Atom**)realloc(pThis->atoms, sizeof(Atom*)*newResidueCount);
    pThis->atoms[newResidueCount-1] = NULL;
    StringArrayAppend(&pThis->residueNames, residueName);
    pThis->atomCount = (int*) realloc(pThis->atomCount, sizeof(int)*newResidueCount);
    pThis->atomCount[newResidueCount-1] = 0;
    resiIndex = AtomParamsSetGetResidueCount(pThis)-1;
  }

  for(atomIndex=0;atomIndex<pThis->atomCount[resiIndex];atomIndex++){
    if(strcmp(pThis->atoms[resiIndex][atomIndex].name, pNewAtom->name) == 0){
      break;
    }
  }
  // new atom
  if(atomIndex == pThis->atomCount[resiIndex]){
    int newAtomCount = pThis->atomCount[resiIndex] + 1;
    pThis->atoms[resiIndex] = (Atom*)realloc(pThis->atoms[resiIndex], sizeof(Atom)*newAtomCount);
    pThis->atomCount[resiIndex]++;
  }

  AtomCopy(&pThis->atoms[resiIndex][atomIndex], pNewAtom);
  //AtomShowParams(&pThis->atoms[resiIndex][atomIndex]);

  return Success;
}

int AtomParamsSetAddFromFile(AtomParamsSet* pThis, char* filePath){
  FileReader file;
  int result;
  char line[MAX_LENGTH_ONE_LINE_IN_FILE+1];
  char usrMsg[MAX_LENGTH_ERR_MSG+1];

  result = FileReaderCreate(&file, filePath);
  if(FAILED(result)){
    sprintf(usrMsg, "in file %s function %s() line %d, when opening: \n%s", __FILE__, __FUNCTION__, __LINE__, filePath);
    TraceError(usrMsg, IOError);
    exit(IOError);
  }

  while(!FAILED(FileReaderGetNextLine(&file, line))){
    Atom atom;
    StringArray params;

    AtomCreate(&atom);
    StringArrayCreate(&params);
    StringArraySplitString(&params, line, ' ');
    char* resiName = StringArrayGet(&params, 0);
    if( resiName == NULL || FAILED(AtomSetParamsByStringArray(&atom, &params)) || FAILED(AtomParamsSetAdd(pThis, resiName, &atom)) ){
      sprintf(usrMsg, "in file %s function %s() line %d, when reading:\n%s", __FILE__, __FUNCTION__, __LINE__, line);
      TraceError(usrMsg, FormatError);
      exit(FormatError);
    }
    StringArrayDestroy(&params);
    AtomDestroy(&atom);
  }

  FileReaderDestroy(&file);
  return Success;
}

int AtomParamsSetGetResidueCount(AtomParamsSet* pThis){
  return StringArrayGetCount(&pThis->residueNames);
}

int AtomParamsSetGetResidueName(AtomParamsSet* pThis, int index, char* residueName){
  char* name = StringArrayGet(&pThis->residueNames, index);
  if(name==NULL){
    return DataNotExistError;
  }
  else{
    strcpy(residueName, name);
    return Success;
  }
}

int AtomParamsSetGetAtomCount(AtomParamsSet* pThis,char* residueName, int* pCount){
  int resiIndex;
  int result;
  result = StringArrayFind(&pThis->residueNames, residueName, &resiIndex);
  if(FAILED(result)){
    return result;
  }
  *pCount = pThis->atomCount[resiIndex];
  return Success;
}

int AtomParamsSetGetAtomParam(AtomParamsSet* pThis,char* residueName, int index, Atom* pDestAtom){
  int resiIndex;
  int result;
  result = StringArrayFind(&pThis->residueNames, residueName, &resiIndex);
  if(FAILED(result)){
    return result;
  }
  if( index<0 || index>=pThis->atomCount[resiIndex] ){
    result = IndexError;
    return result;
  }
  AtomCopy(pDestAtom, &pThis->atoms[resiIndex][index]);
  return Success;  
}

int AtomParamsSetGetAtomParamByName(AtomParamsSet* pThis,char* residueName, char* atomName, Atom* pDestAtom){
  int resiIndex, atomIndex;
  int result = StringArrayFind(&pThis->residueNames, residueName, &resiIndex);
  if(FAILED(result)){
    return result;
  }
  for(atomIndex=0;atomIndex<pThis->atomCount[resiIndex];atomIndex++){
    if(strcmp(pThis->atoms[resiIndex][atomIndex].name, atomName)==0)
      break;
  }
  if(atomIndex==pThis->atomCount[resiIndex]){ // atom not found
    result = DataNotExistError;
    return result;
  }
  AtomCopy(pDestAtom, &pThis->atoms[resiIndex][atomIndex]);
  return Success;
}

int AtomParameterRead(AtomParamsSet* pAtomParam, char* filePath){
  char usrMsg[MAX_LENGTH_ERR_MSG+1];
  if(FAILED(AtomParamsSetAddFromFile(pAtomParam, filePath))){
    sprintf(usrMsg, "in file %s function %s() line %d, when opening:\n%s",__FILE__, __FUNCTION__, __LINE__, filePath);
    TraceError(usrMsg, FormatError);
    exit(FormatError);
  }
  return Success;
}



int AtomparamsSetAssignEEF1Parameters(AtomParamsSet* pThis){
  for(int i = 0; i < AtomParamsSetGetResidueCount(pThis); i++){
    char resiName[MAX_LENGTH_RESIDUE_NAME+1];
    AtomParamsSetGetResidueName(pThis, i, resiName);
    if(!strcmp(resiName, "ALA"))
      ALA_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ARG") == 0)
      ARG_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ASN") == 0)
      ASN_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ASP") == 0)
      ASP_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "CYS") == 0)
      CYS_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLN") == 0)
      GLN_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLU") == 0)
      GLU_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLY") == 0)
      GLY_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "HSD") == 0)
      HSD_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "HSE") == 0)
      HSD_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "HSP") == 0)
      HSD_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ILE") == 0)
      ILE_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "LEU") == 0)
      LEU_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "LYS") == 0)
      LYS_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "MET") == 0)
      MET_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "PHE") == 0)
      PHE_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "PRO") == 0)
      PRO_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "SER") == 0)
      SER_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "THR") == 0)
      THR_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "TRP") == 0)
      TRP_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "TYR") == 0)
      TYR_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "VAL") == 0)
      VAL_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "NTER") == 0)
      NTER_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLYP") == 0)
      GLYP_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "PROP") == 0)
      PROP_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "CTER") == 0)
      CTER_SetEEF1Parameter(pThis->atoms[i], pThis->atomCount[i]);
    else{
      // not defined yet;
    }
  }

  return Success;
}

int AtomparamsSetAssignFOLDEFParameters(AtomParamsSet* pThis){
  for(int i = 0; i < AtomParamsSetGetResidueCount(pThis); i++){
    char resiName[MAX_LENGTH_RESIDUE_NAME+1];
    AtomParamsSetGetResidueName(pThis, i, resiName);
    if(!strcmp(resiName, "ALA"))
      ALA_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ARG") == 0)
      ARG_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ASN") == 0)
      ASN_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ASP") == 0)
      ASP_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "CYS") == 0)
      CYS_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLN") == 0)
      GLN_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLU") == 0)
      GLU_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLY") == 0)
      GLY_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "HSD") == 0)
      HIS_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "HSE") == 0)
      HIS_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "HSP") == 0)
      HIS_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "ILE") == 0)
      ILE_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "LEU") == 0)
      LEU_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "LYS") == 0)
      LYS_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "MET") == 0)
      MET_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "PHE") == 0)
      PHE_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "PRO") == 0)
      PRO_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "SER") == 0)
      SER_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "THR") == 0)
      THR_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "TRP") == 0)
      TRP_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "TYR") == 0)
      TYR_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "VAL") == 0)
      VAL_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "NTER") == 0)
      NTER_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "GLYP") == 0)
      GLYP_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "PROP") == 0)
      PROP_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else if(strcmp(resiName, "CTER") == 0)
      CTER_SetFOLDEFParameter(pThis->atoms[i], pThis->atomCount[i]);
    else{
      // not defined yet;
    }
  }

  return Success;
}
