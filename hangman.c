#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

enum return_codes {
	SUCCESS = 0,
	FILE_ERROR = 1
};

enum game_defaults {
	MAX_WORD_LEN = 64
};

char *generate_default_word_path(void);
void select_word(char *word_path);
int get_linecount(FILE * word_file);

int main(int argc, char *argv[])
// TODO: Everything
{
	if (argc > 2) {
		fprintf(stderr, "Usage: %s [wordsfile]", argv[0]);
	}
	//printf("%zu", strlen(getenv("HOME")));

	char *word_path;
	if (argc == 2) {
		//words_file = argv[1];
	} else {
		word_path = generate_default_word_path();
		select_word(word_path);
		free(word_path);
	}

	return (SUCCESS);
}

char *generate_default_word_path(void)
// Returns a string for the default word path for the current user.
// Returned string must be freed after use.
{
	size_t path_length = (strlen(getenv("HOME")) + 8);	// Length of path required for 
	// $HOME + "/.words" + NULL byte
	char *word_path = (char *)malloc(sizeof(char) * path_length);
	snprintf(word_path, path_length, "%s/.words", getenv("HOME"));
	return (word_path);
}

void select_word(char *word_path)
{

	// Sourced from Liam Echlin in get_random_lyrics.c
	FILE *word_file = fopen(word_path, "r");
	if (!word_file) {
		perror("Could not open words file");
		exit(FILE_ERROR);
	}
	size_t line_count = get_linecount(word_file);

	char *string_list = (char *)calloc(line_count, sizeof(char) * MAX_WORD_LEN);	// 34 is max word length as per spec, rounded up to next power of 2 is 64
	char line_buffer[MAX_WORD_LEN + 2];	// Size of max word length + \n | \0
	for (size_t i = 0; i < line_count; ++i) {
		fgets(line_buffer, (MAX_WORD_LEN + 2), word_file);
		if (line_buffer[0] == '\n') {
			--i;
			continue;
		} else if (line_buffer[strlen(line_buffer) - 1] != '\n'
			   && !feof(word_file)) {
			// Case: Line too long.
			puts("here");
			char consumer = '\0';
			while (consumer != '\n' && consumer != EOF) {
				consumer = getc(word_file);
			}
			--line_count;
			--i;
			continue;
		}
		strncpy(string_list + strlen(string_list), line_buffer,
			strlen(line_buffer));
	}
	printf("String Listing - %s", string_list);
	char **words =
	    (char **)calloc(line_count, sizeof(char *) * MAX_WORD_LEN);
	words[0] = strtok(string_list, "\n");

	for (size_t i = 1; i < line_count; ++i) {
		words[i] = strtok(NULL, "\n");
		printf("Tokenized: %s\n", words[i]);
	}

}

int get_linecount(FILE * word_file)
// Gets count of how many lines of content there are 
// for given file, ignoring duplicate newlines
// and accounting for an absent newline before EOF.
// Returns number of lines with content, valid or invalid.
{
	size_t line_count = 0;
	char consumer = fgetc(word_file);
	if (consumer == EOF) {
		// Case: File is empty
		return (line_count);
	}
	for (;;) {
		// Case: File has content
		if (consumer == '\n') {
			++line_count;
			if ((consumer = fgetc(word_file)) == EOF) {
				break;
			} else if (consumer == '\n') {
				while ((consumer = fgetc(word_file)) == '\n') {
					;
				}
				if (consumer == EOF) {
					break;
				}
			}
		} else if (consumer == EOF) {
			++line_count;
			break;
		}
		consumer = fgetc(word_file);
	}
	// Set pointer to start of file
	fseek(word_file, 0, SEEK_SET);
	return (line_count);
}
