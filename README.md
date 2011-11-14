About
=====

libsentences is a C++ library that splits text into sentences. It learns how
to split sentences by training on a text containing newline separated
sentences. The parameters it learns can be saved and reused later.

### Example text

- Prvi zapisnik odnosi se na sastanak Povjerenstva za radiologiju Ministarstva zdravstva, održan 19. ožujka 2008. Na njemu članovi Povjerenstva, prof. dr. B. Brkljačić, prof. dr. R. Štern-Padovan, prof. dr. Miletić, prof. M. Marotti i drugi ugledni radiolozi upozoravaju predstavnike Ministarstva i HZZO-a na štetnu praksu prepuštanja PET/CT dijagnostike privatnim zdravstvenim ustanovama. Drugi zapisnik odnosi se na sastanak Povjerenstva za radiologiju i Upravnog odbora Hrvatskog društva radiologa, održan u Ministarstvu zdravstva, 18. lipnja 2008. U njemu uz ostalo stoji: “Prof. Brkljačić navodi tvrdnju da PET/CT u RH imaju privatne ustanove, a ne javne ustanove, što predstavlja izvrnutu vrijednost budući da je, primjerice u Austriji, obrnuto”.

### Sentence split text

- Prvi zapisnik odnosi se na sastanak Povjerenstva za radiologiju Ministarstva zdravstva, održan 19. ožujka 2008.
-  Na njemu članovi Povjerenstva, prof. dr. B. Brkljačić, prof. dr. R. Štern-Padovan, prof. dr. Miletić, prof. M. Marotti i drugi ugledni radiolozi upozoravaju predstavnike Ministarstva i HZZO-a na štetnu praksu prepuštanja PET/CT dijagnostike privatnim zdravstvenim ustanovama.
-  Drugi zapisnik odnosi se na sastanak Povjerenstva za radiologiju i Upravnog odbora Hrvatskog društva radiologa, održan u Ministarstvu zdravstva, 18. lipnja 2008.
-  U njemu uz ostalo stoji: “Prof. Brkljačić navodi tvrdnju da PET/CT u RH imaju privatne ustanove, a ne javne ustanove, što predstavlja izvrnutu vrijednost budući da je, primjerice u Austriji, obrnuto”.

Performance
-----------
libsentences processes approximately 500 000 sentences / second using a single
core Intel i5 2500K. Of course, the exact numbers will vary depending on the
corpus that is being processed.

Installation
============
Building from source
--------------------
### Prerequisites
-  [cmake](http://www.cmake.org/)
-  [re2c](http://re2c.org/)
-  [libunistring](http://www.gnu.org/s/libunistring/)
-  [boost](http://www.boost.org/) (headers only)

#### Installing prerequisites on Ubuntu

    sudo apt-get install cmake re2c libunistring-dev libboost-all-dev

#### Installing prerequisites on Mac using [MacPorts](http://www.macports.org/)

    sudo port install cmake re2c libunistring boost

### Build instructions

Type the following commands in your terminal:

    git checkout https://github.com/fsaric/libsentences
    cd libsentences
    mkdir build
    cd build
    cmake ..
    make

Usage
=====

The following example shows how a string can be split into multiple
sentences:

```C++
#include "libsentences/text_sentences.h"
#include "libsentences/sbd_model.h"
#include <iostream>

int main() {
    using namespace libsentences;

    sbd_model model = sbd_model::load("example_model.txt");
    std::string text =
    "Prvi zapisnik odnosi se na sastanak Povjerenstva za radiologiju "
    "Ministarstva zdravstva, održan 19. ožujka 2008. Na njemu članovi "
    "Povjerenstva, prof. dr. B. Brkljačić, prof. dr. R. Štern-Padovan, "
    "prof. dr. Miletić, prof. M. Marotti i drugi ugledni radiolozi "
    "upozoravaju predstavnike Ministarstva i HZZO-a na štetnu praksu "
    "prepuštanja PET/CT dijagnostike privatnim zdravstvenim ustanovama. "
    "Drugi zapisnik odnosi se na sastanak Povjerenstva za radiologiju i "
    "Upravnog odbora Hrvatskog društva radiologa, održan u Ministarstvu "
    "zdravstva, 18. lipnja 2008. U njemu uz ostalo stoji: “Prof. Brkljačić "
    "navodi tvrdnju da PET/CT u RH imaju privatne ustanove, a ne javne "
    "ustanove, što predstavlja izvrnutu vrijednost budući da je, primjerice "
    "u Austriji, obrnuto”.";

    for (auto sentence : text_sentences(text, model)) {
        std::cout << sentence << '\n';
    }
}
```

Compiling (assuming you are in build directory):

    cd ..
    g++ -o example example.cpp -I. build/libsentences.a -lunistring

