
void Ini_Init(const char *buf, int size);

const char *Ini_GetValueFull(const char *var, const char *sec);
int Ini_GetSection(const char *sec, int num);
const char *Ini_GetValue(const char *var);

