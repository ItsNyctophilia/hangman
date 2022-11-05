#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum return_codes {
	SUCCESS = 0,
	USAGE_ERROR = 1,
	FILE_ERROR = 2,
	USER_EOF = 3		// EOF sent by user
};

enum game_defaults {
	MAX_WORD_LEN = 64,	// Maximum word size allowed in the 
	// word_file
	USER_INPUT_BUF = 8	// Arbitrary buffer size; doesn't need
	    // to be longer than 8 chars for input validation
};

struct save_data {
	unsigned long total_games;
	unsigned long wins;
	unsigned long losses;
	unsigned long avg_score;
	unsigned long seconds_played;
};

char *generate_home_path(const char *file_name);
char *select_word(char *word_path, bool default_path_flag);
int get_linecount(FILE * word_file);
int validate_word(char *current_word);
void play_game(char *answer_word);
struct save_data *load_save(void);
void save_game(struct save_data *current_game, unsigned long game_score,
	       unsigned long time_spent);
char *convert_time(unsigned long seconds_played);

int main(int argc, char *argv[])
{
	if (argc > 2) {
		fprintf(stderr, "Usage: %s [word file]\n", argv[0]);
		return (USAGE_ERROR);
	}
	// TODO: All malloc/calloc calls must be error-handled.
	char *word_path;
	char *answer_word;
	bool default_path_flag = true;
	if (argc == 2) {
		// Case: user passed file path
		word_path = argv[1];
		default_path_flag = false;
	} else {
		// Case: default file path
		word_path = generate_home_path("words");
	}
	answer_word = select_word(word_path, default_path_flag);
	if (default_path_flag) {
		free(word_path);
	}
	// DEVPRINT <!>
	printf("%s\n", answer_word);
	// REMOVE BEFORE SUBMISSION
	play_game(answer_word);

	return (SUCCESS);
}

void generate_new_save_file(void)
// Creates a new file for hangman game save data in
// $HOME/.hangman.
{
	char *save_path = generate_home_path("hangman");
	FILE *new_save_file = fopen(save_path, "w");
	if (!new_save_file) {
		// This should only run if the user for some reason
		// has a .words file in their $HOME directory.
		perror("Unable to save game statistics");
		fclose(new_save_file);
		free(save_path);
		exit(FILE_ERROR);
	}
	fprintf(new_save_file, "0,0,0,0,0\n");
	// Format: total_games,wins,losses,avg_score,seconds_played
	free(save_path);
	fclose(new_save_file);
}

char *select_word(char *word_path, bool default_path_flag)
// Uses word_path as file path to read in and valid lines from
// file, then selects one at random. Returns a pointer to a string
// that must be freed after use.
{
	// Sourced from Liam Echlin in get_random_lyrics.c
	FILE *word_file = fopen(word_path, "r");
	if (!word_file) {
		// Case: File does not exist or is not able to be read
		perror("Could not open word file");
		if (default_path_flag) {
			free(word_path);
		}
		exit(FILE_ERROR);
	}
	size_t line_count = get_linecount(word_file);
	char *string_list = calloc(line_count, sizeof(char) * MAX_WORD_LEN);
	size_t validated_line_count = 0;
	char line_buffer[MAX_WORD_LEN + 2];	// Size of max word length + \n + \0
	for (size_t i = 0; i < line_count; ++i) {
		// Iterates through file, ignoring duplicate newlines
		fgets(line_buffer, (MAX_WORD_LEN + 2), word_file);
		if (line_buffer[0] == '\n') {
			// Case: Empty line
			continue;
		} else if (line_buffer[strlen(line_buffer) - 1] != '\n') {
			long int current_position = ftell(word_file);	// Store file 
			//offset to seek back to later if necessary
			if (!
			    (fgets(line_buffer, (MAX_WORD_LEN + 2), word_file)))
			{
				// Case: No newline before EOF
				goto word_validation;
			}
			// Case: Line too long
			fseek(word_file, current_position, 0);	// Seek back to
			//position before test
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

	char **words = calloc(validated_line_count,
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
	free(string_list);
	free(words);
	fclose(word_file);
	return (selected_word);
}

void save_game(struct save_data *current_game, unsigned long game_score,
	       unsigned long time_spent)
{
	char save_string[128] = { '\0' };	// Arbitrary buffer size. The sizes of
	// The individual fields in this file will never naturally get above this
	// allotment unless someone REALLY loves hangman
	++current_game->total_games;
	current_game->seconds_played += time_spent;
	current_game->avg_score =
	    (((current_game->avg_score * (current_game->total_games - 1)) +
	      game_score) / current_game->total_games);
	if (game_score == 5) {
		// Case: player lost
		++current_game->losses;
	} else {
		// Case: player won
		++current_game->wins;
	}
	snprintf(save_string, sizeof(save_string), "%lu,%lu,%lu,%lu,%lu\n",
		 current_game->total_games, current_game->wins,
		 current_game->losses, current_game->avg_score,
		 current_game->seconds_played);

	char *save_path = generate_home_path("hangman");
	FILE *new_save_file = fopen(save_path, "w");
	if (!new_save_file) {
		// This should only run if the user for some reason
		// has a .words file in their $HOME directory.
		perror("Unable to save game statistics");
		fclose(new_save_file);
		free(save_path);
		exit(FILE_ERROR);
	}
	fprintf(new_save_file, "%s", save_string);
	// Format: total_games,wins,losses,avg_score,seconds_played
	free(save_path);
	fclose(new_save_file);

}

struct save_data *load_save(void)
{
	struct save_data *current_game = malloc(sizeof(*current_game));
	// Initializing default values for current_game
	current_game->total_games = 0;
	current_game->wins = 0;
	current_game->losses = 0;
	current_game->avg_score = 0;
	current_game->seconds_played = 0;

	char *save_path = generate_home_path("hangman");
	FILE *save_file = fopen(save_path, "r");
	if (!save_file) {
		// Case: File does not exist or is not able to be read
		generate_new_save_file();
		return (current_game);
	} else {
		// Case: File was able to be read
		char file_buffer[128] = { '\0' };
		// Arbitrary number that should be enough to hold
		// typical save data for hangman in one call of fgets
		int line_count = get_linecount(save_file);
		fgets(file_buffer, sizeof(file_buffer), save_file);
		if (line_count != 1
		    || file_buffer[strlen(file_buffer) - 1] != '\n') {
			// Case: Invalid number of lines or too long of input 
			printf("Unable to read save data from %s. Overwriting.",
			       save_path);
			fclose(save_file);
			free(save_path);
			generate_new_save_file();
			return (current_game);
		}
		char *fields[5];	// Number of fields in struct save_data
		fields[0] = strtok(file_buffer, ",");
		for (size_t i = 1; i < 5; ++i) {
			fields[i] = strtok(NULL, ",\n");
			if (fields[i] == NULL) {
				printf
				    ("Unable to read save data from %s. Overwriting.\n",
				     save_path);
				fclose(save_file);
				free(save_path);
				generate_new_save_file();
				return (current_game);
			}
		}
		for (size_t i = 0; i < 5; ++i) {
			char *ptr = NULL;
			strtoul(fields[i], &ptr, 10);
			if (fields[i] == ptr) {
				printf
				    ("Unable to read save data from %s. Overwriting.\n",
				     save_path);
				fclose(save_file);
				free(save_path);
				generate_new_save_file();
				return (current_game);
			}
		}
		// This is ugly and inefficient,
		// but I honestly cannot think of the syntax 
		// for how to write this better at 2 AM.
		current_game->total_games = strtoul(fields[0], NULL, 10);
		current_game->wins = strtoul(fields[1], NULL, 10);
		current_game->losses = strtoul(fields[2], NULL, 10);
		current_game->avg_score = strtoul(fields[3], NULL, 10);
		current_game->seconds_played = strtoul(fields[4], NULL, 10);
		fclose(save_file);
	}
	free(save_path);
	return (current_game);
}

char *convert_time(unsigned long seconds_played)
// Converts a number in seconds to human readable format
// "HH:MM:SS". Returns output in string form. Resultant
// pointer must be freed after use.
{
	unsigned long seconds = seconds_played;
	unsigned long minutes = 0;
	unsigned long hours = 0;
	while (seconds > 60) {
		seconds -= 60;
		++minutes;
	}
	while (minutes > 60) {
		minutes -= 60;
		++hours;
	}
	// 10 is the length required to fit HHH:MM:SS + '\0'
	char *readable_time = calloc(10, sizeof(char));
	snprintf(readable_time, 10, "%lu:%lu:%lu", hours, minutes, seconds);
	return (readable_time);
}

void play_game(char *answer_word)
{
	size_t word_size = strlen(answer_word);
	// answer_word_state runs parallel to answer_word and is used to
	// toggle on or off the printing of each character
	bool *answer_word_state = calloc(word_size, sizeof(bool));
	bool found_answer;
	char user_input[USER_INPUT_BUF] = { '\0' };
	char seen_guesses[27] = { '\0' };	// length of Alphabet + '\0'
	size_t wrong_guesses = 0;

	struct save_data *before_game = load_save();
	char *readable_time = convert_time(before_game->seconds_played);
	printf("<!> HANGMAN <!>\n");
	printf
	    ("Total games: %lu // Win/Loss: %lu/%lu\n",
	     before_game->total_games, before_game->wins, before_game->losses);
	printf("Avg Score: %lu // Time spent hanging around: %s\n",
	       before_game->avg_score, readable_time);
	free(before_game);
	time_t round_time = time(NULL);
	do {
		printf("%zu - ", wrong_guesses);
		for (size_t i = 0; i < word_size; ++i) {
			if (answer_word_state[i]) {
				printf("%c", *(answer_word + i));
			} else {
				printf("_");
			}
		}
		printf(": ");

		fgets(user_input, sizeof(user_input), stdin);
		if (strlen(user_input) != 2 && !(feof(stdin))) {
			// Case: user input string longer than one char or
			// is newline character
			printf("Invalid input. Guess must be one letter.\n");
			if (strlen(user_input) == USER_INPUT_BUF - 1) {
				// Case: user input string needs 
				//to be flushed from buffer
				int consumer = '\0';
				while (consumer != '\n' && consumer != EOF) {
					consumer = getc(stdin);
				}
			}
		} else if (feof(stdin)) {
			// Case: user entered Ctrl + D to send EOF
			printf("\nExiting. . .\n");
			free(answer_word);
			free(answer_word_state);
			exit(USER_EOF);
		} else {
			user_input[strlen(user_input) - 1] = '\0';	// \n to \0
			char letter = tolower(user_input[0]);
			if (!isalpha(letter)) {
				// Case: user input not in alphabet
				printf
				    ("Invalid input. Guess must be one letter.\n");
			} else {
				// Case: user input passed validation
				if (strchr(seen_guesses, letter)) {
					// Case: duplicate guess
					printf("You already guessed that!\n");
					++wrong_guesses;
				} else {
					seen_guesses[strlen(seen_guesses)] =
					    letter;
					if (!strchr(answer_word, letter)
					    && !strchr(answer_word,
						       toupper(letter))) {
						// Case: letter not in string
						++wrong_guesses;
					} else {
						for (size_t i = 0;
						     i < word_size; ++i) {
							if (letter ==
							    tolower(*
								    (answer_word
								     + i))) {
								// Case: letter is in word
								answer_word_state
								    [i] = true;

							}
						}
					}
				}
			}
		}
		found_answer = true;
		for (size_t i = 0; i < word_size; ++i) {
			if (answer_word_state[i] == false) {
				found_answer = false;
				break;
			}
		}
	} while ((!found_answer) && (wrong_guesses < 5));
	round_time = time(NULL) - round_time;
	if (found_answer) {
		printf("You correctly guessed: %s!\nTotal misses: %zu\n",
		       answer_word, wrong_guesses);
	} else if (wrong_guesses == 5) {
		printf("You lose!\n");
	}
	struct save_data *after_game = load_save();
	save_game(after_game, wrong_guesses, (unsigned long)round_time);
	free(answer_word_state);
	free(answer_word);
}

char *generate_home_path(const char *target_file)
// Returns a string to target_file in current user's $HOME.
// Returned string must be freed after use.
{
	size_t path_length = (strlen(getenv("HOME")) + strlen(target_file) + 3);
	// Length of path required for $HOME + /. + [NAME] + NULL byte
	char *word_path = malloc(sizeof(char) * path_length);
	snprintf(word_path, path_length, "%s/.%s", getenv("HOME"), target_file);
	return (word_path);
}

int validate_word(char *current_word)
// Validates word by converting each char to lowercase
// and comparing it to every alphabetical character.
// Returns 0 if word was invalid, 1 if word was valid.
{
	size_t word_size = strlen(current_word);
	char *lower_current_word = calloc(word_size + 1, sizeof(char));
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
// newline before EOF. Syntax sourced from
// https://stackoverflow.com/a/29752374 from
// pens-fan-69.
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
	// Set stream to start of file
	fseek(word_file, 0, SEEK_SET);
	return (line_count);
}
