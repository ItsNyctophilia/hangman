#include <stdbool.h>
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
char *select_word(char *word_path, bool default_path_flag);
int get_linecount(FILE * word_file);
int validate_word(char *current_word);

int main(int argc, char *argv[])
{
	if (argc > 2) {
		fprintf(stderr, "Usage: %s [wordsfile]", argv[0]);
	}
	//printf("%zu", strlen(getenv("HOME")));

	char *word_path;
	char *answer_word;
	bool default_path_flag = true;
	if (argc == 2) {
		// Case: User passed file path
		word_path = argv[1];
		default_path_flag = false;
	} else {
		// Case: Default file path
		word_path = generate_default_word_path();
	}
	answer_word = select_word(word_path, default_path_flag);
	printf("%s\n", answer_word);
	if (default_path_flag) {
		free(word_path);
	}
	free(answer_word);

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

char *select_word(char *word_path, bool default_path_flag)
{
	// Sourced from Liam Echlin in get_random_lyrics.c
	FILE *word_file = fopen(word_path, "r");

	if (!word_file) {
		// Case: File does not exist or is not able to be read
		perror("Could not open words file");
		free(word_path);
		exit(FILE_ERROR);
	}

	size_t line_count = get_linecount(word_file);
	char *string_list =
	    (char *)calloc(line_count, sizeof(char) * MAX_WORD_LEN);
	size_t validated_line_count = 0;
	char line_buffer[MAX_WORD_LEN + 2];	// Size of max word length + \n | \0
	for (size_t i = 0; i < line_count; ++i) {
		// Iterates through file, ignoring duplicate newlines
		fgets(line_buffer, (MAX_WORD_LEN + 2), word_file);
		if (line_buffer[0] == '\n') {
			// Case: Empty line
			continue;
		} else if (line_buffer[strlen(line_buffer) - 1] != '\n') {
			long int current_position = ftell(word_file);	// Store file offset
			// to seek back to later if necessary
			if (!
			    (fgets(line_buffer, (MAX_WORD_LEN + 2), word_file)))
			{
				// Case: No newline before EOF
				goto word_validation;
			}
			// Case: Line too long
			fseek(word_file, current_position, 0);	// Seek back to position before 
			//test
			int consumer = '\0';
			while (consumer != '\n' && consumer != EOF) {
				consumer = fgetc(word_file);
			}
			continue;
		}
 word_validation:
		if (!(validate_word(line_buffer))) {
			// Case: File failed validation
			continue;
		} else {
			// Case: File passed validation
			++validated_line_count;
			strncpy(string_list + strlen(string_list), line_buffer,
				strlen(line_buffer) + 1);
		}
	}
	if (validated_line_count == 0) {
		// Case: No valid words in file.
		free(string_list);
		if (default_path_flag) {
			free(word_path);
		}
		fclose(word_file);
		fprintf(stderr, "Unable to find valid words in file.\n");
		exit(FILE_ERROR);
	}

	char **words = (char **)calloc(validated_line_count,
				       sizeof(char *) * MAX_WORD_LEN);

	words[0] = strtok(string_list, "\n");
	for (size_t i = 1; i < validated_line_count; ++i) {
		words[i] = strtok(NULL, "\n");
	}

	srand(time(NULL));
	size_t random_index = random() % validated_line_count;

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
	char *lower_current_word = (char *)calloc(word_size + 1, sizeof(char));
	for (size_t i = 0; i < word_size; ++i) {
		lower_current_word[i] = tolower(current_word[i]);
	}
	int success_flag = 0;
	if (lower_current_word[(word_size) - 1] == '\n') {
		// Strip newline if present
		lower_current_word[(word_size) - 1] = '\0';
	}
	if (!
	    (strspn(lower_current_word, "abcdefghijklmnopqrstuvwxyz") ==
	     strlen(lower_current_word))) {
		// Case: Word is invalid
	} else {
		// Case: word is valid
		success_flag = 1;
	}
	free(lower_current_word);
	return (success_flag);
}

int get_linecount(FILE * word_file)
// Counts total lines in file, accounting for no
// newline before EOF.
{
	size_t line_count = 0;
	int chars_on_current_line = 0;
	int consumer;
	//int chars_on_current_line = 0;

	while ((consumer = fgetc(word_file)) != EOF) {
		if (consumer == '\n') {
			++line_count;
			chars_on_current_line = 0;
		} else {
			++chars_on_current_line;
		}
	}
	if (chars_on_current_line > 0) {
		// Case: No newline before EOF
		++line_count;
	}
	// Set pointer to start of file
	fseek(word_file, 0, SEEK_SET);
	return (line_count);
}
