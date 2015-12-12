#define CODE_LENGTH 9
#define EMAIL_LENGTH 32
#define PASSWORD_LENGTH 32
#define ALIAS_LENGTH 32
#define NAME_LENGTH 32

typedef	struct	UserStruct {
	char		email[EMAIL_LENGTH];
	char		password[PASSWORD_LENGTH];
	char		alias[ALIAS_LENGTH];
	char		firstname[NAME_LENGTH];
	char		lastname[NAME_LENGTH];
	char		code[CODE_LENGTH];
}	User;
