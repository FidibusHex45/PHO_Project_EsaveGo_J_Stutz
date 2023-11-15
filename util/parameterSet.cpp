char *errStr = 0;
is_ParameterSet(hCam, IS_PARAMETERSET_CMD_SAVE_FILE, L"../../prop/camConfig.ini", NULL);
int nRet = is_ParameterSet(hCam, IS_PARAMETERSET_CMD_LOAD_FILE, L"../../prop/camConfig.ini", NULL);
is_GetError(hCam, &lastError, &errStr);
std::cerr << "Parameter loaded: " << errStr << std::endl;