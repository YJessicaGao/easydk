#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# EasyDevelopmentKitGuide documentation build configuration file, created by
# sphinx-quickstart on Wed Jun  5 12:46:41 2019.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
import sphinx_rtd_theme

extensions = [
    'breathe',
    'exhale'
]

breathe_projects = {
    "EasyDK": "./doxyoutput/xml"
}
breathe_default_project = "EasyDK"

exhale_args = {
    "containmentFolder":          "./interface",
    "rootFileName":               "Interface_root.rst",
    "rootFileTitle":              "Interface",
    "doxygenStripFromPath":       "../../",
    "fullApiSubSectionTitle":     "API Reference",
    "createTreeView":             False,
    "exhaleExecutesDoxygen":      True,
    "fullToctreeMaxDepth":        3,
    "unabridgedOrphanKinds":      {"dir", "file", "union", "define"},
    "exhaleDoxygenStdin":         """
                                  INPUT = ../../include
                                  INCLUDE_PATH = ../../include
                                  EXCLUDE = ../../include/easyplugin ../../include/internal
                                  EXTRACT_ALL = NO
                                  HIDE_FRIEND_COMPOUNDS = YES
                                  HIDE_UNDOC_MEMBERS = YES
                                  CLANG_ASSISTED_PARSING = YES
                                  CLANG_OPTIONS = -std=c++11
                                  SHOW_NAMESPACES = NO
                                  GENERATE_TREEVIEW = YES
                                  GRAPHICAL_HIERARCHY = YES
                                  INLINE_SIMPLE_STRUCTS = YES
                                  """
}

primary_domain = 'cpp'
highlight_language = 'cpp'

# Add any paths that contain templates here, relative to this directory.
#templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'Index'

# General information about the project.
project = u'EasyDK'
copyright = u'2020, Cambricon'
author = u'Cambricon'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = u'2.4'
# The full version, including alpha/beta/rc tags.
release = u'2.4.0'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = 'en_US'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
# html_theme_options = {}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
#html_static_path = ['_static']

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
html_sidebars = {
    '**': [
        'relations.html',  # needs 'show_related': True theme option to display
        'searchbox.html',
    ]
}


# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'EasyDK Developer Guide'


# -- Options for LaTeX output ---------------------------------------------

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'EasyDK.tex', 'EasyDK Developer Guide',
     'Cambricon', 'manual'),
]


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'EasyDevelopmentKit', 'EasyDevelopmentKit Developer Guide',
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'EasyDevelopmentKit', 'EasyDevelopmentKit Developer Guide',
     author, 'EasyDevelopmentKit', 'One line description of project.',
     'Miscellaneous'),
]

# xelatex 作为 latex 渲染引擎，因为可能有中文渲染
latex_engine = 'xelatex'

# 如果没有使用自定义封面，可以加上封面 logo。这里可以是 pdf 或者 png，jpeg 等
latex_logo = "./images/logo.png"

latex_show_urls = 'footnote'

# 主要的配置，用于控制格式等
latex_elements = {
# The paper size ('letterpaper' or 'a4paper').
#
 'papersize': 'a4paper',

# The font size ('10pt', '11pt' or '12pt').
#
# 'pointsize': '8pt',

'inputenc': '',
'utf8extra': '',

# Additional stuff for the LaTeX preamble.
'preamble': '''

\\addto\\captionsenglish{\\renewcommand{\\chaptername}{}}
% 设置字体

\\usepackage{xeCJK}
\\usepackage{fontspec}
\\setCJKmainfont{Noto Sans CJK SC}
\\setCJKmonofont{Noto Sans CJK SC}
\\setmainfont{Noto Sans CJK SC}

% 段首缩进 2 格

\\usepackage{indentfirst}
\\setlength{\\parindent}{2em}

\\usepackage{setspace}

% 1.5 倍行间距
\\renewcommand{\\baselinestretch}{1.5}
% 表格里的行间距
\\renewcommand{\\arraystretch}{1.5}

% list 列表的缩进对齐
\\usepackage{enumitem}
\\setlist{nosep}

% 表格类的宏包
\\usepackage{threeparttable}
\\usepackage{array}
\\usepackage{booktabs}

% fancy 页眉页脚
\\usepackage{fancyhdr}
\\pagestyle{fancy}

% 在 sphinx 生成的 tex 文件里，normal 是指普通页面（每一个章节里，除了第一页外剩下的页面）
% 页眉，L：left，R：right，E：even，O：odd
% 奇数页面：左边是 leftmark，章号和章名称；右边是 rightmark，节号与节名称
% 偶数页面：左边是 rightmark，节号与节名称；右边是 leftmark，章号和章名称
% textsl 是字体，slanted shape，对于英语而言，某种斜体。但是不同于 textit 的 italic shape
% 左页脚：版权信息
% 右页脚：页码数
% rulewidth：页眉和页脚附近的横线的粗细，当设置为 0pt 时，就没有该横线
%
\\fancypagestyle{normal} {
\\fancyhf{}
\\fancyhead{}
\\fancyhead[LE,RO]{\\textsl{\\rightmark}}
\\fancyhead[LO,RE]{\\textsl{\\leftmark}}
\\lfoot{Copyright © 2019 Cambricon Corporation.}
\\rfoot{\\thepage}
\\renewcommand{\\headrulewidth}{0.4pt}
\\renewcommand{\\footrulewidth}{0.4pt}
}

% 在 sphinx 生成的 tex 文件里，plain 是指每个章节的第一页等
\\fancypagestyle{plain} {
\\fancyhf{}
% left head 还可以内嵌图片，图片可以是 pdf，png，jpeg 等
% \\lhead{\\includegraphics[height=40pt]{cn_tm.pdf}}
\\lhead{\\large\\textcolor[rgb]{0.1804,0.4588,0.7137}{Cambricon™}}
\\lfoot{Copyright © 2019 Cambricon Corporation.}
\\rfoot{\\thepage}
\\renewcommand{\\headrulewidth}{0.4pt}
\\renewcommand{\\footrulewidth}{0.4pt}
}

''',

#
'printindex': r'\footnotesize\raggedright\printindex',

# 移除空白页面
'extraclassoptions': 'openany,oneside',

# 如果需要用 latex 自已做封面，可以使用 maketitle
# 下面这个封面的例子来自于互联网

# 'maketitle': r'''
#         \pagenumbering{Roman} %%% to avoid page 1 conflict with actual page 1
#
#         \begin{titlepage}
#             \centering
#
#             \vspace*{40mm} %%% * is used to give space from top
#             \textbf{\Huge {Sphinx format for Latex and HTML}}
#
#             \vspace{0mm}
#             \begin{figure}[!h]
#                 \centering
#                 \includegraphics[width=0.8\textwidth]{cn.png}
#             \end{figure}
#
#             % \vspace{0mm}
#             % \Large \textbf{{Meher Krishna Patel}}
#
#             % \small Created on : Octorber, 2017
#
#             % \vspace*{0mm}
#             % \small  Last updated : \MonthYearFormat\today
#
#
#             %% \vfill adds at the bottom
#             % \vfill
#             % \small \textit{More documents are freely available at }{\href{http://pythondsp.readthedocs.io/en/latest/pythondsp/toc.html}{PythonDSP}}
#         \end{titlepage}
#
#         \clearpage
#         \pagenumbering{roman}
#         \tableofcontents
#         \listoffigures
#         \listoftables
#         \clearpage
#         \pagenumbering{arabic}
#
#         ''',
#
} # latex_elements






