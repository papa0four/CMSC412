#include <iostream>
#include <string>
#include <string.h>
#include <sys/param.h>
#include <fstream>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <cerrno>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <experimental/filesystem>

#if defined (__ANDROID__)
	#define PLATFORM_NAME "Android"
#elif defined(__linux__)
	#define PLATFORM_NAME "Linux"
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
	#include <sys/param.h>
	#if defined(BSD)
		#define PLATFORM_NAME "BSD"
	#endif
#elif defined(__APPLE__) && defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#define PLATFORM_NAME "iOS"
	#elif TARGET_OS_IPHONE == 1
		#define PLATFORM_NAME "iOS"
	#elif TARGET_OS_MAC == 1
		#define PLATFORM_NAME "osX"
	#endif
#elif defined(__sun) && defined(__SRV4)
	#define PLATFORM_NAME "Solaris"
#else
	#define PLATFORM_NAME NULL
#endif

namespace fs = std::experimental::filesystem;

void list_dir(const char *path)
{
	struct dirent **namelist;
	int i, n;

	n = scandir(".", &namelist, NULL, alphasort);
	if (n < 0)
	{
		perror("scandir");
	}
	else
	{
		for (i = 0; i < n; i++)
		{
			if (PLATFORM_NAME == "Linux" || PLATFORM_NAME == "osX" || PLATFORM_NAME == "BSD")
			{
				if (namelist[i]->d_type == DT_DIR)
				{
					printf("\t\x1B[34m%s\033[0m is a directory\n", namelist[i]->d_name);
				}
				else if (namelist[i]->d_type == DT_REG)
				{
					printf("\t\x1B[32m%s\033[0m is a file\n", namelist[i]->d_name);
				}
			}
			free(namelist[i]);
		}
		free(namelist);
	}
}

bool dir_exists(const char *dir_to_check)
{
	struct stat info;
	if (stat(dir_to_check, &info) != 0)
	{
		return false;
	}
	else if (info.st_mode & S_IFDIR)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void list_dir_recurse(const char* name)
{
	DIR *dir;
	struct dirent *entry;

	if ((dir = opendir(name)) != nullptr)
	{
		while ((entry = readdir(dir)) != nullptr)
		{
			std::string str = std::string(name) + "/" + std::string(entry->d_name);
			if (dir_exists(str.c_str()))
			{
				if (std::string(entry->d_name) != "." && 
										std::string(entry->d_name) != "..")
				{
					
					const char * directory = str.c_str();
					printf("\t%s/\x1B[34m%s\033[0m is a directory\n", name, entry->d_name);
					list_dir_recurse(directory);
				}
				else
				{
					if (entry->d_type == DT_DIR)
					{
						printf("\t%s/\x1B[34m%s\033[0m is a directory\n", name, entry->d_name);
					}
				}
			}
			else
			{
				//std::cout << str.c_str() << std::endl;
				if (entry->d_type == DT_REG)
				{
					printf("\t%s/\x1B[32m%s\033[0m is a file\n", name, entry->d_name);
				}
			}
		}
		closedir(dir);
	}
	else
	{
		std::cout << "Error: " << name << ": No such file or directory\n";
	}
}

void pwd()
{
	char dir[128];
	char *path = getcwd(dir, sizeof(dir));
	if (!path)
	{
		std::cout << "Error: " << std::strerror(errno) << std::endl;
		_Exit(0);
	}
	else
	{
		std::string current;
		current = path;
		std::cout << "\n" << current << "\n" << std::endl;
	}
}

void dump( std::istream& ins )
{
	std::cout << std::setfill( '0' ) << std::hex << std::uppercase;

	std::size_t offset = 0;
	while (ins)
	{
		char s[ 16 ];
		std::size_t n, i;

		std::cout << std::setw( 10 ) << (offset & 0xFFFFFFFFFF) << "  ";
		offset += sizeof(s);

		ins.read( s, sizeof(s) );
		n = ins.gcount();

		for (i = 0; i < n; i++)
		  std::cout << std::setw( 2 ) << (int)s[ i ] << " ";

		while (i++ < sizeof(s))
		  std::cout << "   ";
		std::cout << "  ";

		for (i = 0; i < n; i++)
		{
			if (std::isprint( s[ i ] ))
			{
				std::cout << s[i];
			}
			else
			{
				std::cout << ".";
			}
		}
		std::cout << "\n";
	}
}

char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);

    if(tcgetattr(0, &old) < 0)
    {
        perror("tcsetattr()");
    }
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;

    if(tcsetattr(0, TCSANOW, &old) < 0)
    {
        perror("tcsetattr ICANON");
    }

    if(read(0, &buf, 1) < 0)
    {
        perror("read()");
    }

    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;

    if(tcsetattr(0, TCSADRAIN, &old) < 0)
    {
        perror("tcsetattr ~ICANON");
    }

    return buf;
}

void encrypt_file(std::ifstream& ifs, std::ofstream& ofs,std::string key)
{
	char line[256];

	if (!ofs)
	{
		std::cerr << "Error opening file. " << "\n";
	}

	while (!ifs.eof())
	{
		if (!ifs)
		{
			std::cerr << "\nError opening file. " << " File may not exist.\n\n";
			break;
		}

		ifs.read(line,sizeof(line));

		std::string str_line = line;
		unsigned int j = 0;

		for (unsigned int i = 0; i < str_line.length(); ++i)
		{
			str_line[i] ^= key[j++];
			ofs.put(str_line[i]);
			if (j >= key.length())
			{
				j = 0;
			}
		}
		std::cout << "==================== Encrypted Data =====================\n";
		std::cout << str_line.data() << std::endl;
	}
}

void decrypt_file(std::ifstream& ifs, std::string key)
{
	char line[256];

	while (!ifs.eof())
	{
		if (!ifs)
		{
			std::cerr << "\nError opening file. " << " File may not exist.\n\n";
			break;
		}

		ifs.read(line, sizeof(line));
		
		std::string str_line = line;
		unsigned int j = 0;

		for (unsigned int i = 0; i < str_line.length(); ++i)
		{
			str_line[i] ^= key[j++];
			if (j >= key.length())
			{
				j = 0;
			}
		}
		std::cout << "==================== Decrypted Data ====================\n";
		std::cout << str_line.data() << std::endl;
	}
}

int main() 
{
	int choice;
	bool loop = true;
	while (loop)
	{
		std::string menu_opts[8] = { "Exit", 
								 "Select directory", 
						 		 "List directory content (first level)",
						 		 "List directory content (all levels)",
						 		 "Delete file", 
						 		 "Display file (hexadecimal view)",
						 		 "Encrypt file (XOR with password)",
						 		 "Decrypt file (XOR with password)" 
								};

		menu:
		std::cout << "==================== Homework 5 ====================\n";
		std::cout << "\nPlease enter a number from the following options:\n";
		for (int i = 0; i < 2; i++)
		{
			std::cout << i << " - " << menu_opts[i] << std::endl;
		}
		std::cout << "\n====================================================\n\n";

		std::cout << "User input: ";
		std::cin >> choice;
		switch (choice)
		{
			case 0:
			{
				if (choice == 0)
				{
					std::cout << "Exiting program. Goodbye...\n";
					_Exit(0);
				}
				break;
			}

			case 1:
				std::string buffer;
				char file_path[128];
				if (choice == 1)
				{
					std::cout << "\n====================================================\n";
					std::cout << "\nPlease enter a directory [absolute] filepath.";
					std::cout << "\nOr, enter pwd to display current working directory.\n";
					std::cout << "\n====================================================\n\n";
					std::cout << "User input: ";
					std::cin >> buffer;
					strcpy(file_path, buffer.c_str());
					if (buffer == "pwd")
					{
						pwd();
						goto sub_list;
					}
					else if (0 == chdir(file_path))
					{
						std::cout << "changed directory to: " << getcwd(file_path, sizeof(file_path)) << "\n" << std::endl;
					}
					else
					{
						std::cout << "cd: " << file_path << ": No such file or directory\n\n";
						goto menu;
					}
				}
				sub_list:
					std::cout << "===================== Sub Menu =====================\n\n";
					for (int i = 0; i < 8; i++)
					{
						std::cout << i << " - " << menu_opts[i] << std::endl;
					}
					std::cout << "\n====================================================\n\n";
					std::cout << "User input: ";
					std::cin >> choice;
					switch (choice)
					{
						case 0:
						{
							if (choice == 0)
							{
								std::cout << "Exiting program. Goodbye...\n";
								_Exit(0);
							}
						}

						case 1:
						{
							std::string buffer;
							char file_path[128];
							if (choice == 1)
							{
								std::cout << "\n====================================================\n";
								std::cout << "\nPlease enter a directory [absolute] filepath.";
								std::cout << "\nOr, enter pwd to display current working directory.\n";
								std::cout << "\n====================================================\n\n";
								std::cout << "User input: ";
								std::cin >> buffer;
								strcpy(file_path, buffer.c_str());
								if (buffer == "pwd")
								{
									pwd();
									goto sub_list;
								}
								else if (0 == chdir(file_path))
								{
									std::cout << "changed directory to: " << getcwd(file_path, sizeof(file_path)) << "\n" << std::endl;
								}
								else
								{
									std::cout << "cd: " << file_path << ": No such file or directory\n";
								}
							}
							goto sub_list;
						}

						case 2:
						{
							std::cout << "\n\n================ First Level Output ================\n\n";
							if (buffer == "pwd")
							{
								char dir[128];
								char *path = getcwd(dir, sizeof(dir));
								if (!path)
								{
									std::cout << "Error: " << std::strerror(errno) << std::endl;
								}
								else
								{
									std::cout << "Contents of " << path << ":\n";
									list_dir(path);
								}
							}
							else
							{
								std::cout << "Contents of " << file_path << ":\n";
								list_dir(file_path);
							}
							std::cout << "\n\n====================================================\n\n";
							goto sub_list;
						}

						case 3:
						{
							if (choice == 3)
							{
								char file_path[128];
								char *path = getcwd(file_path, sizeof(file_path));
								if (!path)
								{
									std::cout << "Error: " << std::strerror(errno) << std::endl;
								}
								else
								{
									std::cout << "\n\n================ Recursive Output ================\n\n";
									std::cout << path << std::endl;
									list_dir_recurse(path);
									printf("\n");
									goto sub_list;
								}
							}
						}

						case 4:
						{
							if (choice == 4)
							{
								std::string remove_file;
								std::cout << "Please enter the file you wish to delete: ";
								std::cin >> remove_file;
								if (remove((const char*) remove_file.c_str()) != 0 )
								{
									perror( "Error deleting file. Your file may not exist within this directory." );
								}
								else
								{
								    std::cout << remove_file << " successfully deleted\n";
								}
								char dir[128];
								char *path = getcwd(dir, sizeof(dir));
								pwd();
								list_dir(path);
							}
							goto sub_list;
						}

						case 5:
						{
							if (choice == 5)
							{
								std::string view_file;
								std::cout << "Please enter the file you wish to view in hex: ";
								std::cin >> view_file;
								std::ifstream  inp_file(view_file.c_str(), std::ios::binary);
								if (!inp_file)
								{
									std::cerr << "Error opening file: " << view_file << " File may not exist.\n";
								}
								else
								{
									std::cout << "\n\n===================== Hex View =====================\n\n";
									dump(inp_file);
									std::cout << "\n\n====================================================\n\n";
								}
								inp_file.close();
							}
							goto sub_list;
						}

						case 6:
						{
							std::string in_file;
							std::string code;
							std::string pass;
							char key;

							if (choice == 6)
							{
								std::cout << "Please enter a password (upto 256 bytes to encrypt your file.\n";
								std::cout << "Note: you may use any combination of letters, numbers, and/or special characters.\n";
								std::cout << "It is a good idea to remember this password for decryption of your file.\n";
								std::cout << "And, as always, this password will be case sensitive...\n";

								while ((key = getch()) != 10)
								{
									std::cout << '*';
									pass = std::to_string(key);
								}

								std::cout << "\nPlease enter the name of the file you wish to encrypt: ";
								std::cin >> in_file;
								std::cout << "Now enter the name of the file you wish to create containing the encrypted data: ";
								std::cin >> code;

								std::ifstream ifs(in_file.c_str(), std::ios::binary);
								std::ofstream ofs(code.c_str());

								encrypt_file(ifs, ofs, pass);

								ofs.close();
								ifs.close();

								std::ifstream ifstm(code);
								if (!ifstm)
								{
									std::cerr << "Error opening file: " << code << "\n";
								}
							}
							goto sub_list;
						}

						case 7:
						{
							std::string in_file;
							std::string pass;
							char key;

							if (choice == 7)
							{
								std::cout << "Please enter the same password used previously for decryption purposes.\n";

								while ((key = getch()) != 10)
								{
									std::cout << '*';
									pass = std::to_string(key);
								}

								std::cout << "\nPlease enter the name of the encrypted file you wish to dencrypt: ";
								std::cin >> in_file;

								std::ifstream ifs(in_file.c_str());
								if (ifs.fail())
								{
									std::cerr << "Error opening file: " << in_file << " File may not exist.\n";
								}

								decrypt_file(ifs, pass);

								ifs.close();								
							}
							goto sub_list;
						}

						default:
						{
							std::cout << "Invalid option selected.\n";
							std::cout << "Exiting program. Goodbye...\n";
							_Exit(0);
						}
					}
			}
		}
		loop = false;
	

	return 0;
}