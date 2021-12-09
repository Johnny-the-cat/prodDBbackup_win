#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

bool parseCmdLine(int argc, char* argv[], char *login, char *pass, char *dblink, char **dumpdir, char **select,  char **jsonreportfile, bool *consistent)
{
	const char *usage = "Usage: prodDBbackup login/pass@dblink dumpdir=C:\\dir\\to\\backup [schemaset=\"Select ....\" \
jsonreportfile=C:\\path\\to\\file consistent=no|yes]\n\
consistent is equivalent \"flashback_time=systimestamp\" option for expdp, \"no\" is default\n\
For multithread export create PRODDBBACKUP_DIR_1, PRODDBBACKUP_DIR_2... directories on your DB server\n\
prodDBbackup version - 1.3.1";

	if (argc < 3 || argc > 6)
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

	bool gotdumpdir = FALSE;

	bool gotselect = FALSE;

	bool gotjsonreportfile = FALSE;

	bool gotconsistent = FALSE;

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
		
		else if (strstr(argv[i], "jsonreportfile=") == argv[i])
		{
			if (gotjsonreportfile == FALSE)
			{
				*jsonreportfile = argv[i] + (int)strlen("jsonreportfile=");
				if (strlen(*jsonreportfile) != 0)
				{
					gotjsonreportfile = TRUE;
					continue;
				}
			}
			else
			{
				printf("Argument \"jsonreportfile\" is alredy got\n");
				printf("%s\n", usage);
				exit(EXIT_FAILURE);
			}
		}

		else if (strstr(argv[i], "consistent=") == argv[i])
		{
			if (gotconsistent == FALSE)
			{
				char tmpstr[5] = { 0 };
				strncpy (tmpstr, argv[i] + (int)strlen("consistent="), 4);
				int i;
				for (i = 0; tmpstr[i] != 0; i++)
				{
					tmpstr[i] = tolower(tmpstr[i]);
				}
				if (strcmp (tmpstr, "yes") == 0)
				{
					*consistent = TRUE;
					gotconsistent = TRUE;
					continue;
				}
			}
			else
			{
				printf("Argument \"consistent\" is alredy got\n");
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

	if (gotconsistent == FALSE)
	{
		//по-умолчанию консистентность отключена
		*consistent = FALSE; 
		gotconsistent = TRUE;
	}

	return TRUE;
}