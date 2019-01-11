#include "ob/parg.hh"
using Parg = OB::Parg;

#include "ob/term.hh"
namespace aec = OB::Term::ANSI_Escape_Codes;

#include <cstddef>

#include <string>
#include <iostream>

// prototypes
int program_options(Parg& pg);

int program_options(Parg& pg)
{
  pg.name("nansi").version("0.1.1 (10.01.2019)");
  pg.description("A CLI tool to filter ansi escape sequences and format text streams.");
  pg.usage("[flags] [options] [--] [arguments]");
  pg.usage("[-v|--version]");
  pg.usage("[-h|--help]");
  pg.info("Examples", {
    "nansi --help",
    "nansi --version",
  });
  pg.info("Exit Codes", {"0 -> normal", "1 -> error"});
  pg.info("Repository", {
    "https://github.com/octobanana/nansi.git",
  });
  pg.info("Homepage", {
    "https://octobanana.com/software/nansi",
  });
  pg.author("Brett Robinson (octobanana) <octobanana.dev@gmail.com>");

  // general flags
  pg.set("help,h", "print the help output");
  pg.set("version,v", "print the program version");

  pg.set("ansi", "allow ansi escape sequences");
  pg.set("line-wrap", "wrap lines at custom width or terminal width");
  pg.set("auto-wrap", "wrap lines and auto calculate the indent width");
  pg.set("first-wrap", "if the indentation level is 0, don't wrap the line");
  pg.set("white-space", "sequences of whitespace will collapse into a single whitespace");

  pg.set("width", "0", "num", "maximum output width");

  pg.set_stdin();

  int status {pg.parse()};

  if (status > 0 && pg.get_stdin().empty())
  {
    std::cerr << pg.help() << "\n";
    std::cerr << "Error: " << "expected arguments" << "\n";

    return -1;
  }

  if (status < 0)
  {
    std::cerr << pg.help() << "\n";
    std::cerr << "Error: " << pg.error() << "\n";

    auto similar_names = pg.similar();
    if (similar_names.size() > 0)
    {
      std::cerr
      << "did you mean:\n";
      for (auto const& e : similar_names)
      {
        std::cerr
        << "  --" << e << "\n";
      }
    }

    return -1;
  }

  if (pg.get<bool>("help"))
  {
    std::cerr << pg.help();

    return 1;
  }

  if (pg.get<bool>("version"))
  {
    std::cerr << pg.name() << " v" << pg.version() << "\n";

    return 1;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  Parg pg {argc, argv};
  int pstatus {program_options(pg)};
  if (pstatus > 0) return 0;
  if (pstatus < 0) return 1;

  try
  {
    OB::Term::ostream os {std::cout};

    os.line_wrap(false);

    if (pg.get<bool>("line-wrap") || pg.get<bool>("auto-wrap"))
    {
      os.line_wrap(true);
      os.auto_wrap(pg.get<bool>("auto-wrap"));

      if (pg.get<bool>("first-wrap"))
      {
        os.first_wrap(false);
      }

      if (pg.find("width"))
      {
        os.width(pg.get<std::size_t>("width"));
      }
      else
      {
        if (OB::Term::is_term(STDOUT_FILENO))
        {
          os.width(OB::Term::width(STDOUT_FILENO));
        }
      }
    }

    if (! pg.get<bool>("ansi"))
    {
      os.escape_codes(false);
    }

    if (! pg.get<bool>("white-space"))
    {
      os.white_space(false);
    }

    os
    << pg.get_stdin()
    << OB::Term::iomanip::flush();
  }
  catch(std::exception const& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
  }
  catch(...)
  {
    std::cerr << "Error: an unexpected error occurred\n";
  }

  return 0;
}
