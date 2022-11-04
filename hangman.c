#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

enum return_codes {
	SUCCESS = 0,
	FILE_ERROR = 1
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
	char *word_path = (char *)malloc(path_length);
	snprintf(word_path, path_length, "%s/.words", getenv("HOME"));
	return (word_path);
}

void select_word(char *word_path)
{

	// Sourced from Liam Echlin in get_random_lyrics.c
	FILE *word_file;
	if ((word_file = fopen(word_path, "r")) == NULL) {
		perror("Could not open words file");
		exit(FILE_ERROR);
	}
	int line_count = get_linecount(word_file);
	//char *valid word = "words";
	for (int i = 0; i < line_count; ++i) {

	}

	printf("%d <-- lines", line_count);
}

int get_linecount(FILE * word_file)
// Gets count of how many lines of content there are 
// for given file, ignoring duplicate newlines
// and accounting for an absent newline before EOF.
// Returns number of lines with content, valid or invalid.
{
	int line_count = 0;
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
	return (line_count);
}
