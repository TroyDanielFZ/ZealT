# Inspirations

Thanks for the [zealdocs/zeal: Offline documentation browser inspired by Dash](https://github.com/zealdocs/zeal), and I used to use it a lot for documentation browsering.
However, I find the built-in browser frustrate me a lot. I'm a vim user, and in browser, I use Surfingkyes to complete most of the actions.
Therefore, I modify the original Zeal so that I'm used to.

The modifications are mainly in:
* [-] Remove the built-in browser.
* [-] Remove configuration dialog, config by editing the "zeal.ini" file directly.
* [+] Support "AND" logical search, namely, for example, the search term "str find" will show the items contains "str" and "find" at the same time.
* [+] Open the result with the custom software.
* [+] PDF documentations are supported.
* [+] Customizable file type association



# Custom association 


Syntax: 
```
[associations]
filetypes=pdf,html
pdf_regex=\\.pdf$|\\.pdf%7C
pdf_spliter=%7C
pdf_command="\"e:/GreenSoftware/Office/PDFEditor/PDFXEdit.exe\"  /A nameddest=\"\"\"{$2}\"\"\"\"\" \"{$1}\""
html_regex=\\.html?$|\\.html?#
html_command=e:/GreenSoftware/Internet/centbrowser/chrome.exe -url \"{%}\"
```

Explanation:
1. At the very beginning, the filetypes are given at [associations/filetypes], splitted by comma, for example, there are two types are defined, e.g., `pdf` and `html`.
2. Then, there are several information for each association:
	a) `*_regex` or `*_match`: To determine if the current file matches the type or not, if matches, this program will use the corresponding comand defined later. If both are defined, `*_regex` is used, e.g. `*_match` is ignored.
	b) `*_command`: define the command to be executed, with several macros available.
	c) `*_splitter`: define a splitter string to be used and explained later.
3. The filetype judgement follows the order defined by [associations/filetypes], if one of the pattern is matched, the rest are ignored.

## `*_regex` and `*_match`

The program use these two on the full path to judge the filetype. The full path is in the uri scheme, for example, the filepath in the database is `Command	The Comprehensive LATEX Symbol List.pdf|Mathematical symbols` with the docset stored in `e:\Code\ZealT\docsets\software.docset\`, the obtained filepath is `file:///e:/Code/ZealT/docsets/software.docset/Contents/Resources/Documents/The Comprehensive LATEX Symbol List.pdf%7CMathematical symbols`. Please note that, the `|` is escaped as `%7C`.

For `*_regex`, the program use regex expression for judgement. To be more specific, use `Regex.indexIn(...) != -1`.

For `*_regex`, the program use wildchar mode for judgement. To be more specific, first `Regex.setPatternSyntax(QRegExp::Wildcard);` and then use `rx.exactMatch(strFilepath)`.

For example, to match a word documenta, you can use the following configuration:
```ini
word_match=*.doc?
word_command="\"c:\Program Files\Microsoft Office\root\Office16\WINWORD.EXE\" \"{%}\""
```

## `*_splitter`

The program use the splitter defined by `*_splitter` (default: `%7C` is not given) to split the uri into several groups. 

## `*_command`

The macro for the command line are:

| macros                      | Explanation                                                                                                                                                            |
| --------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `{%}`, `{$0}`               | the full uri, for example: `file:///e:/Code/ZealT/docsets/software.docset/Contents/Resources/Documents/The Comprehensive LATEX Symbol List.pdf%7CMathematical symbols` |
| `{$1}`, `{$2}`, ..., `{$9}` | different group                                                                                                                                                        |


Example:

```
pdf_command="\"e:/GreenSoftware/Office/PDFEditor/PDFXEdit.exe\"  /A nameddest=\"\"\"{$2}\"\"\"\"\" \"{$1}\""
```

In batch console, we can use 
```
e:/GreenSoftware/Office/PDFEditor/PDFXEdit.exe /A nameddest="Mathematical symbols" "e:/Code/ZealT/docsets/software.docset/Contents/Resources/Documents/The Comprehensive LATEX Symbol List.pdf"
```
to locate the bookmark "Mathematical symbols" in file "e:/Code/ZealT/docsets/software.docset/Contents/Resources/Documents/The Comprehensive LATEX Symbol List.pdf". 

To avoid the quotes to be incorrectly placed when called by `QProcess.startDetached()`, we should escape the quote in the midlle of a parameter, namely, after the `nameddest=`, and the corresponding one. The escaped version is:
```
e:/GreenSoftware/Office/PDFEditor/PDFXEdit.exe /A nameddest="""Mathematical symbols""" "e:/Code/ZealT/docsets/software.docset/Contents/Resources/Documents/The Comprehensive LATEX Symbol List.pdf"
```

In the database, the path is given as `The Comprehensive LATEX Symbol List.pdf|Mathematical symbols`.
Then uri obtained is `file:///e:/Code/ZealT/docsets/software.docset/Contents/Resources/Documents/The Comprehensive LATEX Symbol List.pdf%7CMathematical symbols`, with the defined seperator `%7C`, two groups are obtained:
1. `file:///e:/Code/ZealT/docsets/software.docset/Contents/Resources/Documents/The Comprehensive LATEX Symbol List.pdf`
2. `Mathematical symbols`

Again, to generate a quote read by `QSettings`, the quote should be escaped with `\`. Therefore, we get the final `pdf_command`.




# License
This software is licensed under the terms of the GNU General Public License version 3 (GPLv3), following the origial license. Full text of the license is available in the [COPYING](COPYING) file and [online](https://www.gnu.org/licenses/gpl-3.0.html).
