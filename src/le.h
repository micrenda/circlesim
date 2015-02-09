int le_init();
int le_loadexpr(char *expr, char **pmsg);
void le_eval(int cookie, char **pmsg);
double le_eval_double(int cookie, char **pmsg);
int    le_eval_integer(int cookie, char **pmsg);
int    le_eval_boolean(int cookie, char **pmsg);
void le_unref(int cookie);
void le_setvar(char *name, double value);
double le_getvar(char *name);
