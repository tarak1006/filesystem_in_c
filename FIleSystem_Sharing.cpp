#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<string.h>
#include<io.h>
#define BLOCK_SIZE 128 //
#define MasterBlock_start 0//128 bytes
#define UserInfo_start 128// 20 blocks//hex 80
#define BitVector_start 2688// 256 blocks=32768//hex A80
#define MessageMetadata_start 35456//32768 blocks=4194304bytes//hex 8A80
#define MessageInfo_start 4229760//32768 blocks=4194304bytes//hex 408A80
#define FileBitVector_start 8424064//184320bytes//HEX 808A80
#define FileMetaData_start 8608384//2048 blocks//HEX 835A80
#define FileInfo_start 8870528  //HEX 875A80
//2099968


typedef struct {
	long long int userinfo_start;
	long long int bitvector_start;
	long long int message_metadata_start;
	long long int message_info_start;
	long long int user_count;
	long long int file_metadata_count;
	long long int filemetadata_start;
	long long int fileinfo_start;
	int pad[16];

}MasterBlock;


typedef struct {
	char user_name[16];
	char password[16];
	long long int MessageMetadataStart_position;
	int pad[22];
}UserInfo;


typedef struct {
	char bit_vector[128];
}BitVector;

typedef struct {
	char user_name[16];
	char file_name[16];
	long long int message_startposition;
	long long int message_count;
	int pad[20];
	//int buffer[4]
}MessageMetadata;

typedef struct{
	char msg[128];
	//long long int message_nextposition;
	//long long int message_endvalue;
}MessageInfo;


typedef struct{

	int file_bitvector[32];

}FileBitVector;
typedef struct{
	long long int file_id;
	char file_name[64];

	long long int file_size;
	long long int filesize_blocks;
	int pad[10];
}FileMetaData;

typedef struct{
	char data[2048];

}FileInfo;


void* allocate(){
	return calloc(1, BLOCK_SIZE);
}

void* read_block(FILE *fp, int offset){
	fseek(fp, offset, SEEK_SET);
	void* memory = allocate();
	fread(memory, BLOCK_SIZE, 1, fp);
	return memory;
}
void write_block(FILE *fp, int offset, void* memory){
	fseek(fp, offset, SEEK_SET);
	fwrite(memory, BLOCK_SIZE, 1, fp);
	fclose(fp);
	fp = fopen("FileSystem.dat", "rb+");

}

long long int calculate_from_messageinfo(FILE *fp, int no_of_blocks){
	MessageInfo* message_info = (MessageInfo*)calloc(1, sizeof(MessageInfo));
	printf("\n enter message :");
	fflush(stdin);
	scanf("%[^\n]",message_info->msg);
	
	//gets(message_info->msg);
	//message_info->message_nextposition = -1;
	fseek(fp, MessageInfo_start + (128 * no_of_blocks), SEEK_SET);
	long long int message_info_position = ftell(fp);
	write_block(fp, ftell(fp), message_info);
	return message_info_position;

}

int calculate_from_metadata(FILE *fp, char* username, int no_of_blocks){
	char file_name[40];
	MessageMetadata* message_metadata = (MessageMetadata*)calloc(1, sizeof(MessageMetadata));
	fseek(fp, MessageMetadata_start + (128 * no_of_blocks), SEEK_SET);
	long long int message_metadata_position = ftell(fp);
	strcpy(message_metadata->user_name, username);
	printf("\n enter file name :");
	scanf("%s", message_metadata->file_name);
	//strcpy(message_metadata->file_name, file_name);
	//printf("\n in metadata user name %s", message_metadata->user_name);
	message_metadata->message_startposition = calculate_from_messageinfo(fp, no_of_blocks);
	//message_metadata->message_count = 1;

	write_block(fp, message_metadata_position, message_metadata);

	return 1;

}

void calculate_from_bitvector(FILE *fp, char* username){
	int no_of_metadata_blocks = 0;
	int no_of_bitvectors = 0;
	BitVector* read_bit_vector = (BitVector*)calloc(1, sizeof(BitVector));
	int i;
	fseek(fp, BitVector_start, SEEK_SET);
	while (no_of_bitvectors < 256){
		fseek(fp, BitVector_start + (128 * no_of_bitvectors), SEEK_SET);
		read_bit_vector = (BitVector*)read_block(fp, ftell(fp));
		i = 0;
		while (read_bit_vector->bit_vector[i] == '1' && i < 128){
			i++;
		}
		if (i < 128){
			no_of_metadata_blocks += i;
			break;
		} 
		
	
		no_of_metadata_blocks += i;
		no_of_bitvectors++;
	}
	read_bit_vector->bit_vector[i] = '1';
	calculate_from_metadata(fp, username, no_of_metadata_blocks);

	write_block(fp, BitVector_start+(no_of_bitvectors*128), read_bit_vector);

	
	return;


}


void read_message(FILE *fp){
	int no_of_bitvectors = 0;
	int index = 0;
	char user_name[30];
	char file_name[30];
	//printf("\n enter user name to read :");
	//scanf("%s", user_name);
	printf("\n enter file name to read :");
	scanf("%s", file_name);
	fseek(fp, BitVector_start, SEEK_SET);
	BitVector* read_bit_vector = new BitVector();
	MessageMetadata* read_mesg_metadata = new MessageMetadata();
	MessageInfo* read_mesg_info = new MessageInfo();
	fseek(fp, BitVector_start, SEEK_SET);
	while (no_of_bitvectors < 256){
		fseek(fp, BitVector_start + (128 * no_of_bitvectors), SEEK_SET);
		read_bit_vector = (BitVector*)read_block(fp, ftell(fp));
		int i;
		for (i = 0; read_bit_vector->bit_vector[i] != '0' && i < 128; i++){
			fseek(fp, MessageMetadata_start + (128 * index), SEEK_SET);
			//printf("\n actual %d original %d", MessageMetadata_start, ftell(fp));
			read_mesg_metadata = (MessageMetadata*)read_block(fp, ftell(fp));
			if (strcmp(read_mesg_metadata->file_name, file_name) == 0){
				fseek(fp, read_mesg_metadata->message_startposition, SEEK_SET);
				read_mesg_info = (MessageInfo*)read_block(fp, ftell(fp));
				printf("\n message read is : %s", read_mesg_info->msg);

			}
			index++;
		}
		if (read_bit_vector->bit_vector[i] == '0')
			break;
		no_of_bitvectors++;
	}
}

void read_all_messages_filesystem(FILE *fp){
	int no_of_bitvectors = 0;
	int index = 0;
	fseek(fp, BitVector_start, SEEK_SET);
	BitVector* read_bit_vector = new BitVector();
	MessageMetadata* read_mesg_metadata = new MessageMetadata();
	MessageInfo* read_mesg_info = new MessageInfo();
	fseek(fp, BitVector_start, SEEK_SET);
	while (no_of_bitvectors < 256){
		fseek(fp, BitVector_start + (128 * no_of_bitvectors), SEEK_SET);
		read_bit_vector = (BitVector*)read_block(fp, ftell(fp));
		int i;
		for (i = 0; read_bit_vector->bit_vector[i] != '0'&& i < 128; i++){
			fseek(fp, MessageMetadata_start + (128 * index), SEEK_SET);
			//printf("\n actual %d original %d", MessageMetadata_start, ftell(fp));
			read_mesg_metadata = (MessageMetadata*)read_block(fp, ftell(fp));
			fseek(fp, read_mesg_metadata->message_startposition, SEEK_SET);
			read_mesg_info = (MessageInfo*)read_block(fp, ftell(fp));
			printf("\n message read is  : %s", read_mesg_info->msg);
			index++;

		}
		if (read_bit_vector->bit_vector[i] == '0')
			break;
		no_of_bitvectors++;
	}
}

void delete_message(FILE *fp){
	int flag = 0;
	int index = 0;
	char file_name[40];
	char message[128];
	int no_of_bitvectors = 0;
	printf("\n enter file name :");
	scanf("%s", file_name);
	printf("\n enter message :");
	fflush(stdin);
	scanf("%[^\n]", message);

	//scanf("%s", message);
	fseek(fp, BitVector_start, SEEK_SET);
	BitVector* read_bit_vector = new BitVector();
	MessageMetadata* read_mesg_metadata = new MessageMetadata();
	MessageInfo* read_mesg_info = new MessageInfo();
	fseek(fp, BitVector_start, SEEK_SET);

	while (no_of_bitvectors < 256){
		fseek(fp, BitVector_start + (128 * no_of_bitvectors), SEEK_SET);
		read_bit_vector = (BitVector*)read_block(fp, ftell(fp));
		int i;
		for (i = 0; read_bit_vector->bit_vector[i] != '0' && i < 128; i++){

			fseek(fp, MessageMetadata_start + (128 * index), SEEK_SET);
			//printf("\n actual %d original %d", MessageMetadata_start, ftell(fp));
			read_mesg_metadata = (MessageMetadata*)read_block(fp, ftell(fp));
			if (strcmp(read_mesg_metadata->file_name, file_name) == 0){
				fseek(fp, read_mesg_metadata->message_startposition, SEEK_SET);
				read_mesg_info = (MessageInfo*)read_block(fp, ftell(fp));
				if (strcmp(read_mesg_info->msg, message) == 0){
					read_bit_vector->bit_vector[i] = '2';
					//printf("\n message read is %s", read_mesg_info->msg);
					strcpy(read_mesg_metadata->file_name, "");
					fseek(fp, MessageMetadata_start + (128 * index), SEEK_SET);
					write_block(fp, ftell(fp), read_mesg_metadata);
					fseek(fp, BitVector_start + (no_of_bitvectors * 128), SEEK_SET);
					write_block(fp, ftell(fp), read_bit_vector);
					flag = 1;
					break;

				}
			}
			index++;
		}
		if (flag == 1)
			break;
		no_of_bitvectors++;

	}
	}


void delete_all_messages_of_file(FILE *fp,char* file_name){
	int flag = 0;
	int index = 0;
	//char file_name[40];
	char message[128];
	int no_of_bitvectors = 0;
	//printf("\n enter file name :");
	//scanf("%s", file_name);
	//printf("\n enter message :");
	//scanf("%s", message);
	fseek(fp, BitVector_start, SEEK_SET);
	BitVector* read_bit_vector = new BitVector();
	MessageMetadata* read_mesg_metadata = new MessageMetadata();
	MessageInfo* read_mesg_info = new MessageInfo();
	fseek(fp, BitVector_start, SEEK_SET);

	while (no_of_bitvectors < 256){
		fseek(fp, BitVector_start + (128 * no_of_bitvectors), SEEK_SET);
		read_bit_vector = (BitVector*)read_block(fp, ftell(fp));
		int i;
		for (i = 0; read_bit_vector->bit_vector[i] != '0' && i < 128; i++){

			fseek(fp, MessageMetadata_start + (128 * index), SEEK_SET);
			//printf("\n actual %d original %d", MessageMetadata_start, ftell(fp));
			read_mesg_metadata = (MessageMetadata*)read_block(fp, ftell(fp));
			if (strcmp(read_mesg_metadata->file_name, file_name) == 0){
				fseek(fp, read_mesg_metadata->message_startposition, SEEK_SET);
				read_mesg_info = (MessageInfo*)read_block(fp, ftell(fp));
			
					read_bit_vector->bit_vector[i] = '2';
					//printf("\n message read is %s", read_mesg_info->msg);
					strcpy(read_mesg_metadata->file_name, "");
					fseek(fp, MessageMetadata_start + (128 * index), SEEK_SET);
					write_block(fp, ftell(fp), read_mesg_metadata);
					fseek(fp, BitVector_start + (no_of_bitvectors * 128), SEEK_SET);
					write_block(fp, ftell(fp), read_bit_vector);
					//flag = 1;
					//break;

				
			}
			index++;
		}
		if (i<128 && read_bit_vector->bit_vector[i] == '0')
			break;
		no_of_bitvectors++;

	}
}




void create_file_info(FILE *fp,int index,char* memory){

	fseek(fp, FileInfo_start + (2048 * index), SEEK_SET);
	fwrite(memory, 2048, 1, fp);
	fclose(fp);
	fp = fopen("FileSystem.dat", "rb+");



}

int Create_file_bitvector(FILE *fp, int id,char* file_name,long long int totalsize){
	int no_of_file_bitvector_blocks = 0, index = 0, flag = 0;
	FILE *upload_fp = fopen(file_name, "rb");
	int count_blocks = 0;
	fseek(fp, FileBitVector_start, SEEK_SET);
	//int dummy_count = 0;
	char buffer[2048];
	int i;

	
	int size = (totalsize / 2048) + 1;
	int remaining_bytes = totalsize % 2048;
	fseek(upload_fp, 0, SEEK_SET);
	while (no_of_file_bitvector_blocks < 1440){
		fseek(fp, FileBitVector_start + (128 * no_of_file_bitvector_blocks), SEEK_SET);
		FileBitVector* read_file_bitvector = (FileBitVector*)read_block(fp, ftell(fp));
		for (i = 0; i < 32; i++, index++){
			//printf("%d", index);
			if (read_file_bitvector->file_bitvector[i] == 0 || read_file_bitvector->file_bitvector[i] == -1){
				if (size - 1 == count_blocks){
					flag = 1;
					read_file_bitvector->file_bitvector[i] = id;
					break;
				}
				fread(buffer, 2048, 1, upload_fp);
				//printf("\n\n%s", buffer);
				count_blocks++, create_file_info(fp, index, buffer), read_file_bitvector->file_bitvector[i] = id;


			}
		}
		
		fseek(fp, FileBitVector_start + (128 * no_of_file_bitvector_blocks), SEEK_SET);
		write_block(fp, ftell(fp), read_file_bitvector);
		if (flag == 1){
			
			fread(buffer, remaining_bytes, 1, upload_fp), create_file_info(fp, index, buffer);
			break;
			//printf("\n=====%s", buffer);
			
			
		}
			
		no_of_file_bitvector_blocks++;
	}
	return size;

}
void create_file_metadata(FILE *fp,int no_of_ids){
	int id = (no_of_ids+1);
	char file_name[40];
	printf("\n enter file name :");
	scanf("%s", file_name);
	FILE *upload_fp = fopen(file_name, "rb");
	

	fseek(upload_fp, 0, SEEK_END);

	//printf("\n%d", ftell(upload_fp));
	long long int totalsize = ftell(upload_fp);
	FileMetaData* read_file_metadata = new FileMetaData();
	int final_position;
	fseek(fp, FileMetaData_start, SEEK_SET);
	do{
		
		read_file_metadata = (FileMetaData*)read_block(fp, ftell(fp));


	} while (read_file_metadata->file_id != -1);
	fseek(fp, -128, SEEK_CUR);
	final_position = ftell(fp);
	
	strcpy( read_file_metadata->file_name,file_name);
	read_file_metadata->file_size = totalsize;
	read_file_metadata->file_id = id;
	read_file_metadata->filesize_blocks = Create_file_bitvector(fp,id,read_file_metadata->file_name,totalsize);

	write_block(fp, final_position, read_file_metadata);

	return;


}


void read_file_through_bitvector(FILE *fp, int id, int no_of_blocks,long long int totalsize){
	int no_of_file_bitvector_blocks = 0, index = 0, flag = 0;
	char destination[40];
	printf("\n enter destination file name :");
	scanf("%s", destination);
	FILE *write_fp = fopen(destination, "wb");
	int remaining_bytes = totalsize % 2048;
	int count_blocks = 0;
	fseek(fp, FileBitVector_start, SEEK_SET);

	char buffer[2048];
	int i;
	while (no_of_file_bitvector_blocks < 1440){
		fseek(fp, FileBitVector_start + (128 * no_of_file_bitvector_blocks), SEEK_SET);
		FileBitVector* read_file_bitvector = (FileBitVector*)read_block(fp, ftell(fp));
		for (i = 0; i < 32; i++, index++){
			if (count_blocks == no_of_blocks-1){
				flag = 1;
				break;
			}
			if (read_file_bitvector->file_bitvector[i] == id){
				count_blocks++;
				fseek(fp, FileInfo_start + (2048 * index), SEEK_SET);
				fread(&buffer, 2048, 1, fp);

				//printf("\n\n %s\n\n", buffer);

				fwrite(&buffer, 2048, 1, write_fp);


			}
		}

		if (flag == 1)
			break;
		no_of_file_bitvector_blocks++;
	}
	char* buffer2 = (char *)malloc(remaining_bytes*sizeof(char));
	fseek(fp, FileInfo_start + (2048 * index), SEEK_SET);
	fread(buffer2, remaining_bytes, 1, fp);

	//printf("\n\n %s\n\n", buffer2);

	fwrite(buffer2, remaining_bytes, 1, write_fp);
	fclose(write_fp);


}
void read_file_metadata(FILE *fp){
	char file_name[40];
	char destination[40];
	
	printf("\n enter file name :");
	scanf("%s", file_name);
	

	FileMetaData* read_file_metadata = new FileMetaData();
	int final_position;
	fseek(fp, FileMetaData_start, SEEK_SET);
	do{

		read_file_metadata = (FileMetaData*)read_block(fp, ftell(fp));
		if (strcmp(read_file_metadata->file_name, file_name) == 0){
			break;
		}


	} while (read_file_metadata->file_id != -1);
	
	
	read_file_through_bitvector(fp, read_file_metadata->file_id, read_file_metadata->filesize_blocks,read_file_metadata->file_size);
	
	
	return;






}
void delete_file_bitvector(FILE *fp, int id, int blocks){
	int no_of_file_bitvector_blocks = 0;
	int count = 0, flag = 0, index = 0, i;
	fseek(fp, FileBitVector_start, SEEK_SET);
	while (no_of_file_bitvector_blocks < 1440){
		fseek(fp, FileBitVector_start + (128 * no_of_file_bitvector_blocks), SEEK_SET);
		FileBitVector* read_file_bitvector = (FileBitVector*)read_block(fp, ftell(fp));
		for (i = 0; i < 32; i++){

			if (count == blocks){
				flag = 1;
				break;
			}
			if (read_file_bitvector->file_bitvector[i] == id){
				read_file_bitvector->file_bitvector[i] = -1;
				count++;
			}
		}
		fseek(fp, FileBitVector_start + (128 * no_of_file_bitvector_blocks), SEEK_SET);
		write_block(fp, ftell(fp), read_file_bitvector);
		if (flag == 1)
			break;
		no_of_file_bitvector_blocks++;
	}
}

void delete_file(FILE *fp){
	char file_name[40];
	printf("\n enter file name :");
	scanf("%s", file_name);
	long long int final_position;
	FileMetaData* read_file_metadata = new FileMetaData();
	//int final_position;
	fseek(fp, FileMetaData_start, SEEK_SET);
	do{

		read_file_metadata = (FileMetaData*)read_block(fp, ftell(fp));
		if (strcmp(read_file_metadata->file_name, file_name) == 0){
			
			break;
		}


	} while (read_file_metadata->file_id != -1);
	fseek(fp, -128, SEEK_CUR);
	final_position = ftell(fp);
	delete_file_bitvector(fp, read_file_metadata->file_id, read_file_metadata->filesize_blocks);

	strcpy(read_file_metadata->file_name, "");
	read_file_metadata->file_size = 0;
	read_file_metadata->file_id = -1;
	delete_all_messages_of_file(fp,file_name);

	write_block(fp, final_position, read_file_metadata);
	return;


}






void main_menu(FILE *fp, char* username){
	int option2 = 0;
	
	MasterBlock* read_master_block = (MasterBlock*)read_block(fp, 0);
	while (1){
		printf("\n==Perform Operation===\n");
		printf("\n 1.writemessage 2.readmessage 3.delete message 4.upload_file 5.download_file  6.delete file  7.exit\n");
		scanf("%d", &option2);
		switch (option2){
		case 1:calculate_from_bitvector(fp, username); break;

		case 2:read_message(fp); break;


		case 3:delete_message(fp); break;


			//read_file_metadata(file_pointer);

			//	read_master_block->file_metadata_count++;

		case 4:create_file_metadata(fp, read_master_block->file_metadata_count);
			read_master_block->file_metadata_count++;
			break;

		case 5:read_file_metadata(fp); break;


		case 6:delete_file(fp); break;
		default:break;




		}
		if (option2 == 7)
			break;
	}
	write_block(fp, 0, read_file_metadata);


}


int CreateUser(FILE *fp, int usercount){
	UserInfo* user_info = (UserInfo*)calloc(1, sizeof(UserInfo));
	printf("\n enter user name :");
	scanf("%s", user_info->user_name);
	printf("\n enter user password :");
	scanf("%s", user_info->password);
	//calculate_from_bitvector(fp, user_info->user_name);
	main_menu(fp, user_info->user_name);
	write_block(fp, UserInfo_start + (128 * usercount), user_info);

	return 1;
}

int SearchUser(FILE *fp, int usercount){
	fseek(fp, UserInfo_start, SEEK_SET);
	char username[30];
	if (usercount == 0){
		printf("\n you have to sign up first...");
		return CreateUser(fp, usercount);
	}
	else{
		printf("\n enter user name  :");
		scanf("%s", username);
		char password[30];
		for (int i = 0; i < usercount; i++){
			fseek(fp, UserInfo_start + (128 * i), SEEK_SET);
			UserInfo* user_info = (UserInfo*)calloc(1, sizeof(UserInfo));
			user_info = (UserInfo*)read_block(fp, ftell(fp));
			if (strcmp(user_info->user_name, username) == 0){
				//printf("\n you are already a valid user");
				printf("\n enter your password :");
				scanf("%s", password);
				if (strcmp(user_info->password, password) == 0){
					printf("\n Sucessfully logged in.");
					//calculate_from_bitvector(fp, username);
					main_menu(fp, username);
					return 0;
				}
				else{
					printf("\n Invalid Password...");
					return 0;
				}

			}
		}
		printf("\n you have to sign up first :)");
		return CreateUser(fp, usercount);
	}




}




int main(void) {
	/* initialize empty array with the right size */
	int control = 1;
	if ((_access("FileSystem.dat", 0)) == -1)
	//if (control==1)
	{
		printf("\n Intially file created on your system.so.please run again.this takes some time.....");
		int no_of_bitvectors = 0;
		FILE *file_pointer = fopen("FileSystem.dat", "wb+");
		file_pointer = fopen("FileSystem.dat", "rb+");
		MasterBlock* master_block = (MasterBlock*)calloc(1, sizeof(MasterBlock));
		master_block->userinfo_start = UserInfo_start;
		master_block->bitvector_start = BitVector_start;
		master_block->message_info_start = MessageInfo_start;
		master_block->message_metadata_start = MessageMetadata_start;
		master_block->user_count = 0;
		master_block->filemetadata_start = FileMetaData_start;
		master_block->fileinfo_start = FileInfo_start;
		master_block->file_metadata_count = 0;
		BitVector* read_bit_vector = new BitVector();



		fseek(file_pointer, BitVector_start, SEEK_SET);
		while (no_of_bitvectors < 256){
			fseek(file_pointer, BitVector_start + (128 * no_of_bitvectors), SEEK_SET);
			read_bit_vector = (BitVector*)read_block(file_pointer, ftell(file_pointer));
			int i;
			for (i = 0; i < 128; i++){
				read_bit_vector->bit_vector[i] = '0';
			}
			fseek(file_pointer, BitVector_start + (128 * no_of_bitvectors), SEEK_SET);
			write_block(file_pointer, ftell(file_pointer), read_bit_vector);
			no_of_bitvectors++;
		}

		int no_of_file_bitvectors = 0;
		FileBitVector* read_file_bit_vector = new FileBitVector();

		fseek(file_pointer, FileBitVector_start, SEEK_SET);
		while (no_of_file_bitvectors < 1440){
			fseek(file_pointer, FileBitVector_start + (128 * no_of_file_bitvectors), SEEK_SET);
			read_file_bit_vector = (FileBitVector*)read_block(file_pointer, ftell(file_pointer));
			int i;
			for (i = 0; i < 32; i++){
				read_file_bit_vector->file_bitvector[i] = 0;
			}
			fseek(file_pointer, FileBitVector_start + (128 * no_of_file_bitvectors), SEEK_SET);
			write_block(file_pointer, ftell(file_pointer), read_file_bit_vector);
			no_of_file_bitvectors++;
		}


		int no_of_file_metadata=0;
		FileMetaData* read_file_metadata = new FileMetaData();
		fseek(file_pointer, FileMetaData_start, SEEK_SET);
		while (no_of_file_metadata < 2048){
			fseek(file_pointer, FileMetaData_start + (128 * no_of_file_metadata), SEEK_SET);
			read_file_metadata = (FileMetaData*)read_block(file_pointer, ftell(file_pointer));

			read_file_metadata->file_id = -1;

			fseek(file_pointer, FileMetaData_start + (128 * no_of_file_metadata), SEEK_SET);
			write_block(file_pointer, ftell(file_pointer), read_file_metadata);
			no_of_file_metadata++;
		}

			   






		write_block(file_pointer, 0, master_block);
		
		fclose(file_pointer);
	}
	else{
		FILE *file_pointer = fopen("FileSystem.dat", "rb+");
		MasterBlock* master_block = (MasterBlock*)calloc(1, sizeof(MasterBlock));
		UserInfo* user_info = (UserInfo*)calloc(1, sizeof(UserInfo));
		BitVector* bit_vector = (BitVector*)calloc(1, sizeof(BitVector));
		MessageInfo* message_info = (MessageInfo*)calloc(1, sizeof(MessageInfo));
		MessageMetadata* message_metadata = (MessageMetadata*)calloc(1, sizeof(MessageMetadata));
		char username[30];
		MasterBlock* read_master_block = (MasterBlock*)read_block(file_pointer, 0);
		//delete_message(file_pointer);
		int user_count = read_master_block->user_count;
		printf("\n=====WELCOME TO FILE SYSTEM==========\n");
		int option;
		printf("\n 1.Login 2.SignUp\n");
		scanf("%d", &option);
		switch (option){
		case 1:if (SearchUser(file_pointer, read_master_block->user_count) == 1){
			user_count += 1;
		} break;
		case 2:CreateUser(file_pointer, read_master_block->user_count);
			user_count++; break;
		default:break;
		}
	
		read_master_block = (MasterBlock*)read_block(file_pointer, 0);
		read_master_block->user_count = user_count;
		write_block(file_pointer, 0, read_master_block);
		fclose(file_pointer);

		return 0;
	}
}