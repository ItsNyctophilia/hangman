#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

enum return_codes {
	SUCCESS = 1
};

char *generate_default_word_path(void);

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
		// TODO: generate word
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
