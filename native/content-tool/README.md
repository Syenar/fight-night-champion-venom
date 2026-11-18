# Content helper attribution

fnc_content.cpp and its original CMake recipe come from the FNC Reborn TU2 reference source (https://github.com/bjarnimaron/FNCReborn-TU2), based on FNC Reborn by Whompay (https://github.com/whompay/FNCReborn). The source README identifies Apache License 2.0 and requests preservation of its NOTICE attribution. This copy credits both projects; the reference snapshot supplied to this workspace contains no LICENSE or NOTICE file. No license or notice text has been invented.

The CMake SDK requirement is changed to 1.0.0.0 EXACT for the pinned SDK baseline. build-content-tool.py adds a portable, receipt-producing direct build against that same SDK source/library pair. The helper reads caller-owned packages and decodes caller-owned speech assets. It contains no game media or generated title code.

The base FNC Reborn upstream LICENSE and NOTICE supplied with the bundled APT helpers are preserved here verbatim. They identify FNC Reborn by Whompay and do not cover game files or generated title code. The TU2 fork credit above remains explicit.
