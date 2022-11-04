#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum return_codes {
	SUCCESS = 0,
	FILE_ERROR = 1
};

enum game_defaults {
	MAX_WORD_LEN = 64
};

char *generate_default_word_path(void);
char *select_word(char *word_path);
int get_linecount(FILE * word_file);
int validate_word(char *current_word);

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
		char *answer_word = select_word(word_path);

		printf("%s\n", answer_word);
		free(answer_word);
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

char *select_word(char *word_path)
{ 
	// Sourced from Liam Echlin in get_random_lyrics.c
	FILE *word_file = fopen(word_path, "r");
	if (!word_file) {
		perror("Could not open words file");
		exit(FILE_ERROR);
	}
	size_t line_count = get_linecount(word_file);

	char *string_list =
	    (char *)calloc(line_count, sizeof(char) * MAX_WORD_LEN);
	char line_buffer[MAX_WORD_LEN + 2];	// Size of max word length + \n | \0
	for (size_t i = 0; i < line_count; ++i) {
		// Iterates through file, ignoring duplicate newlines
		fgets(line_buffer, (MAX_WORD_LEN + 2), word_file);
		if (line_buffer[0] == '\n') {
			--i;
			continue;
		} else if (line_buffer[strlen(line_buffer) - 1] != '\n'
			   && !feof(word_file)) {
			// Case: Line too long.
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
	char **words =
	    (char **)calloc(line_count, sizeof(char *) * MAX_WORD_LEN);

	words[0] = strtok(string_list, "\n");
	for (size_t i = 1; i < line_count; ++i) {
		words[i] = strtok(NULL, "\n");
	}
	for (size_t i = 0; i < line_count; ++i) {
		if (!(validate_word(words[i]))) {
			for (size_t x = i; x < line_count; ++x) {
				words[x] = words[x + 1];
			}
			--line_count;
			--i;
		}
	}
	srand(time(NULL));
	size_t random_index = random() % line_count;
	char *selected_word =
	    (char *)calloc(strlen(words[random_index]) + 1, sizeof(char));
	strncpy(selected_word, words[random_index],
		strlen(words[random_index]));
	// Free all allocated memory
	free(string_list);
	free(words);
	fclose(word_file);
	return (selected_word);
}

int validate_word(char *current_word)
// Validates word by converting each char to lowercase
// and comparing it to every alphabetical character.
// Returns 0 if word was invalid, 1 if word was valid.
{
	size_t word_size = strlen(current_word);
	char *lower_current_word =
	    (char *)calloc(strlen(current_word) + 1, sizeof(char));
	for (size_t i = 0; i < word_size; ++i) {
		lower_current_word[i] = tolower(current_word[i]);
	}
	int success_flag = 0;

	for (size_t i = 0; i < word_size; ++i) {
		if (!
		    (strspn(lower_current_word, "abcdefghijklmnopqrstuvwxyz") ==
		     word_size)) {
			// Case: Word is invalid
		} else {
			if (current_word[(word_size) - 1] == '\n') {
				// Strip newline if present
				current_word[(word_size) - 1] = '\0';
			}
			// Case: word is valid
			success_flag = 1;
		}
	}
	free(lower_current_word);
	return (success_flag);
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