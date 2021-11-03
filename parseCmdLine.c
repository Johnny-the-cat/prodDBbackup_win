#include <stdio.h>
#include <string.h>
#include <windows.h>

BOOL parseCmdLine(int argc, char* argv[], char *login, char *pass, char *dblink, char **dumpdir, char **select)
{
	const char *usage = "Usage: prodDBbackup login/pass@dblink dumpdir=C:\\dir\\to\\backup [schemaset=\"Select ....\"]\n\
for multithread export create PRODDBBACKUP_DIR_1, PRODDBBACKUP_DIR_2... directories on your DB server\n\
prodDBbackup version - 1.1.2";

	if (argc < 3 || argc > 4)
	{
		printf("%s\n", usage);
		exit(EXIT_FAILURE);
	}
	int i;


	char *slashpointer, *atpointer;
	int connectlinelen;

	connectlinelen = strlen(argv[1]);
	//printf("Connect line lengh - %d\n", connectlinelen);

	slashpointer = strchr(argv[1], '/');
	atpointer = strchr(argv[1], '@');
	if ((slashpointer - argv[1]) == 0 || *(argv[1]) == '/')
	{
		printf("Login is not found\n");
		printf("%s\n", usage);
		exit(EXIT_FAILURE);
	}

	if (slashpointer == NULL || (atpointer - slashpointer - 1) == 0)
	{
		printf("Pass is not found\n");
		printf("%s\n", usage);
		exit(EXIT_FAILURE);
	}

	if (atpointer == NULL || (connectlinelen - (atpointer - argv[1]) - 1) == 0)
	{
		printf("dblink is not found\n");
		printf("%s\n", usage);
		exit(EXIT_FAILURE);
	}

	if ((slashpointer - argv[1]) > 30)
	{
		printf("Login is too large\n");
		exit(EXIT_FAILURE);
	}

	if (atpointer - slashpointer - 1 > 30)
	{
		printf("Pass is too large\n");
		exit(EXIT_FAILURE);
	}

	if ((connectlinelen - (atpointer - argv[1]) - 1) > 127)
	{
		printf("dblink is too large\n");
		exit(EXIT_FAILURE);
	}

	strncpy_s(login, 32, argv[1], slashpointer - argv[1]);

	strncpy_s(pass, 32, slashpointer + 1, atpointer - slashpointer - 1);

	strncpy_s(dblink, 128, atpointer + 1, connectlinelen - (atpointer - argv[1]) - 1);

	BOOL gotdumpdir = FALSE;

	BOOL gotselect = FALSE;

	for (i = 2; i < argc; i++)
	{
		if (strstr(argv[i], "dumpdir=") == argv[i])
		{
			if (gotdumpdir == FALSE)
			{
				*dumpdir = argv[i] + (int)strlen("dumpdir=");
				if (strlen(*dumpdir) != 0)
				{
					gotdumpdir = TRUE;
					continue;
				}
			}
			else
			{
				printf("Argument \"dumpdir\" is alredy got\n");
				printf("%s\n", usage);
				exit(EXIT_FAILURE);
			}
		}

		else if (strstr(argv[i], "schemaset=") == argv[i])
		{
			if (gotselect == FALSE)
			{
				*select = argv[i] + (int)strlen("schemaset=");
				if (strlen(*select) != 0)
				{
					gotselect = TRUE;
					continue;
				}
			}
			else
			{
				printf("Argument \"schemaset\" is alredy got\n");
				printf("%s\n", usage);
				exit(EXIT_FAILURE);
			}
		}
		
		else
		{
			printf("Unknown argument\n");
			printf("%s\n", usage);
			exit(EXIT_FAILURE);
		}
	}

	if (gotdumpdir == FALSE)
	{
		printf("Argument dumpdir is missed\n");
		printf("%s\n", usage);
		exit(EXIT_FAILURE);
	}

	if (gotselect == FALSE)
	{
		*select = "select username from dba_users where default_tablespace not like 'SYSTEM' and default_tablespace not like 'SYSAUX' and username not like '%_IMAGE' and username not like '%!_KO' ESCAPE '!' and account_status not like 'EXPIRED _ LOCKED' order by username";
		gotselect = TRUE;
	}

	return TRUE;
}