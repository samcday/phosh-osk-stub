/*
 * Copyright (C) 2022 Guido Günther <agx@sigxcpu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-completer"

#include "pos-config.h"

#include "pos-completer.h"
#include "pos-completer-priv.h"
#include "util.h"

/**
 * PosCompleter:
 *
 * Interface for completion engines.
 *
 * Completion engines implement this interface so they can be used by
 * the OSK to complete or correct user input. Users of this interface
 * should fill the [property@Completer:preedit] with user input and
 * will get a list of possible completions in the `completions`
 * property. Note that this can happen asynchronously as getting the
 * completions can take time.
 *
 * The completer can also instruct the user of this interface to
 * commit a given text via the [signal@Completer::commit-string]
 * signal. Implementations should emit this signal on word breaking
 * characters to either take the user input as is or force
 * "aggressive" autocorrection (picking a correction on the users
 * behalf).
 */

G_DEFINE_INTERFACE (PosCompleter, pos_completer, G_TYPE_OBJECT)

/* TODO: all the brackets, also language dependent, tab, etc */
static const char * const completion_end_symbols[] = {
  " ",
  ".",
  ",",
  ";",
  ":",
  "?",
  "!",
  "(", ")",
  "{", "}",
  "[", "]",
  NULL,
};


GQuark
pos_completer_error_quark (void)
{
  static GQuark quark = 0;

  if (!quark)
    quark = g_quark_from_static_string ("pos-completer");

  return quark;
}


void
pos_completer_default_init (PosCompleterInterface *iface)
{
  GType iface_type = G_TYPE_FROM_INTERFACE (iface);

  /**
   * PosCompleter:name:
   *
   * The name of this completer
   */
  g_object_interface_install_property (
    iface, g_param_spec_string ("name", "", "", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  /**
   * PosCompleter:preedit:
   *
   * The preedit is not yet submitted text at the current cursor position.
   */
  g_object_interface_install_property (
    iface, g_param_spec_string ("preedit", "", "", NULL, G_PARAM_READWRITE));

  /**
   * PosCompleter:before-text:
   *
   * The text before the current cursor position. The completer has to
   * make sense of it e.g. by parsing backwards for the last separation char.
   * No guarantee is made that it contains the start of a sentence.
   * It should be used as context for better completions.
   */
  g_object_interface_install_property (
    iface, g_param_spec_string ("before-text", "", "", NULL, G_PARAM_READABLE));

  /**
   * PosCompleter:after-text:
   *
   * The text after the current cursor position. The completer has to
   * make sense of it e.g. by parsing forward for the next word end.
   */
  g_object_interface_install_property (
    iface, g_param_spec_string ("after-text", "", "", NULL, G_PARAM_READABLE));

  /**
   * PosCompleter:completions:
   *
   * The list of completions for a given preedit.
   */
  g_object_interface_install_property (
    iface, g_param_spec_boxed ("completions", "", "", G_TYPE_STRV, G_PARAM_READABLE));

  /**
   * PosCompleter::commit-string:
   * @iface: The completer interface
   * @string: The commit string
   *
   * The completer wants the given text to be committed as is. This can
   * happen when the completer encounters a word separating character
   * (e.g. space). `preedit` will be set to empty in this case.
   */
  g_signal_new ("commit-string",
                iface_type,
                G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL,
                G_TYPE_NONE,
                1,
                G_TYPE_STRING);
  /**
   * PosCompleter::update
   * @iface: The completer interface
   * @string: The new preedit string
   * @before: Number of bytes before the cursor to delete
   * @after: Number of bytes after the cursor to delete
   *
   * The completer changed it's preedit to the given string
   * and wants the given number of bytes before and after
   * the preedit removed.
   *
   * The completer must emit the signal before the corresponding `notify::preedit`
   * TODO: add marshallers.
   */
  g_signal_new ("update",
                iface_type,
                G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL,
                G_TYPE_NONE,
                3,
                G_TYPE_STRING,
                G_TYPE_UINT,
                G_TYPE_UINT);
}


/**
 * pos_completer_feed_symbol:
 * @self: The completer
 * @symbol: The symbol to process.
 *
 * Feeds a symbol to be processed by the completer.
 *
 * Returns: %TRUE if the symbol was processed, %FALSE otherwise.
 */
gboolean
pos_completer_feed_symbol (PosCompleter *self, const char *symbol)
{
  PosCompleterInterface *iface;

  g_return_val_if_fail (POS_IS_COMPLETER (self), FALSE);

  iface = POS_COMPLETER_GET_IFACE (self);
  g_return_val_if_fail (iface->feed_symbol != NULL, FALSE);
  return iface->feed_symbol (self, symbol);
}

/**
 * pos_completer_get_completions:
 * @self: the completer
 *
 * Returns the current possible completions.
 *
 * Returns: (transfer none): the completions
 */
GStrv
pos_completer_get_completions (PosCompleter *self)
{
  GStrv completions;

  g_return_val_if_fail (POS_IS_COMPLETER (self), NULL);
  g_object_get (self, "completions", &completions, NULL);

  return completions;
}

/**
 * pos_completer_get_preedit:
 * @self: the completer
 *
 * Returns the current preedit
 *
 * Returns: (transfer none): the current preedit
 */
const char *
pos_completer_get_preedit (PosCompleter *self)
{
  PosCompleterInterface *iface;

  g_return_val_if_fail (POS_IS_COMPLETER (self), NULL);

  iface = POS_COMPLETER_GET_IFACE (self);
  g_return_val_if_fail (iface->get_preedit != NULL, NULL);

  return iface->get_preedit (self);
}

/**
 * pos_completer_set_preedit:
 * @self: the completer
 * @preedit: the preedit text to set
 *
 * Sets the current preedit. The preedit is the current word under
 * completion.
 */
void
pos_completer_set_preedit (PosCompleter *self, const char *preedit)
{
  PosCompleterInterface *iface;

  g_return_if_fail (POS_IS_COMPLETER (self));

  iface = POS_COMPLETER_GET_IFACE (self);
  g_return_if_fail (iface->set_preedit != NULL);

  return iface->set_preedit (self, preedit);
}

/**
 * pos_completer_get_before_text:
 * @self: the completer
 *
 * Returns the current [property@Pos.Completer:before-text].
 *
 * Returns: (transfer none): the current before_text
 */
const char *
pos_completer_get_before_text (PosCompleter *self)
{
  PosCompleterInterface *iface;

  g_return_val_if_fail (POS_IS_COMPLETER (self), NULL);

  iface = POS_COMPLETER_GET_IFACE (self);
  /* optional */
  if (iface->get_before_text == NULL)
    return "";

  return iface->get_before_text (self);
}

/**
 * pos_completer_get_after_text:
 * @self: the completer
 *
 * Returns the current [property@Pos.Completer:after-text].
 *
 * Returns: (transfer none): the current after_text
 */
const char *
pos_completer_get_after_text (PosCompleter *self)
{
  PosCompleterInterface *iface;

  g_return_val_if_fail (POS_IS_COMPLETER (self), NULL);

  iface = POS_COMPLETER_GET_IFACE (self);
  /* optional */
  if (iface->get_after_text == NULL)
    return "";

  return iface->get_after_text (self);
}


/**
 * pos_completer_set_surrounding_text:
 * @self: the completer
 * @before_text: the text before the cursor
 * @after_text: the text after the cursor
 *
 * Set the text before and after the current cursor position. This can
 * be used by the completer to improve the prediction.
 */
void
pos_completer_set_surrounding_text (PosCompleter *self,
                                    const char   *before_text,
                                    const char   *after_text)
{
  PosCompleterInterface *iface;

  g_return_if_fail (POS_IS_COMPLETER (self));

  iface = POS_COMPLETER_GET_IFACE (self);
  /* optional */
  if (iface->set_surrounding_text == NULL)
    return;

  return iface->set_surrounding_text (self, before_text, after_text);
}

/* Used by completers to simplify implenetations */

/**
 * pos_completer_add_preedit:
 * @self: the completer
 * @preedit: The current preedit
 * @symbol: the symbol to add
 *
 * Adds the current symbol to preedit.
 *
 * Returns: %TRUE if preedit should be submitted as is. %FALSE otherwise.
 */
gboolean
pos_completer_add_preedit (PosCompleter *self, GString *preedit, const char *symbol)
{
  gboolean is_ws;

  g_return_val_if_fail (POS_IS_COMPLETER (self), FALSE);
  g_return_val_if_fail (symbol, FALSE);

  if (g_strcmp0 (symbol, "KEY_BACKSPACE") == 0 && preedit->len) {
    /* Remove last utf-8 character */
    const char *last = &preedit->str[preedit->len];
    const char *prev = g_utf8_find_prev_char (preedit->str, last);
    int n = last - prev;
    g_string_truncate (preedit, preedit->len - n);
    return FALSE;
  }

  if (g_strcmp0 (symbol, "KEY_ENTER") == 0) {
    g_string_append (preedit, "\n");
    return TRUE;
  }

  if (g_str_has_prefix (symbol, "KEY_"))
    return FALSE;

  g_string_append (preedit, symbol);

  if (pos_completer_symbol_is_word_separator (symbol, &is_ws)) {
    if (is_ws == FALSE)
      g_string_append (preedit, " ");
    return TRUE;
  }

  return FALSE;
}

/**
 * pos_completer_symbol_is_word_separator:
 * @symbol: the symbol to check
 * @is_ws:(out) (nullable): whether symbol is a whitespace
 *
 * Checks if the given symbol is a word separator like a full stop,
 * exclamation mark, etc. `is_ws` will be set to %TRUE if it's a
 * whitespace character.
 *
 * Returns: %TRUE if the sumbol is a word separator. %FALSE otherwise.
 */
gboolean
pos_completer_symbol_is_word_separator (const char *symbol,
                                        gboolean   *is_ws)
{
  /* TODO: use hash table - or rather just use is_alnum? */
  for (int i = 0; i < g_strv_length ((GStrv)completion_end_symbols); i++) {
    if (strcmp (symbol, completion_end_symbols[i]) == 0) {
      if (i == 0 && is_ws != NULL)
        *is_ws = TRUE;
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * pos_completer_grab_last_word:
 * @text: the text to grab the last word from
 * @new_text:(out): The new text with the last word removed
 * @word: The last word of text
 *
 * Scans `text` from the end returns the last word. If `text` ends with
 * whitespace the last word is considered empty and `new_text` and `word`
 * remain unchanged.
 *
 * Returns: %TRUE `new_text` and `word` were filled.
 */
gboolean
pos_completer_grab_last_word (const char *text, char **new_text, char **word)
{
  gsize len;
  g_autofree char *symbol = NULL;

  g_return_val_if_fail (new_text && *new_text == NULL, FALSE);
  g_return_val_if_fail (word && *word == NULL, FALSE);

  /* Nothing to parse */
  if (STR_IS_NULL_OR_EMPTY (text))
    return FALSE;

  /* text ends with whitespace */
  len = g_utf8_strlen (text, -1);
  symbol = g_utf8_substring (text, len-1, len);
  if (pos_completer_symbol_is_word_separator (symbol, NULL))
    return FALSE;

  /* Get last word in text */
  for (glong start = len - 1; start >= 0; start--) {
    g_free (symbol);
    symbol = g_utf8_substring (text, start, start+1);

    if (pos_completer_symbol_is_word_separator (symbol, NULL)) {
      *word = g_strdup (g_utf8_offset_to_pointer (text, start+1));
      *new_text = g_utf8_substring (text, 0, start+1);
      return TRUE;
    }
  }

  /* Now whitespace in text */
  *new_text = NULL;
  *word = g_strdup (text);

  return TRUE;
}
