v 0.x (plan)
================================================================================
- TODO:
    - Simple Opening Book
    - IID check
    - WIN MSC 
        // GTEST - nice examples
        // Windows:  _MSC_VER
        // _BitScanForward64/_BitScanReverse64
        // https://docs.microsoft.com/en-us/cpp/intrinsics/bitscanreverse-bitscanreverse64?view=vs-2019
    - IDEA: (credit to Robert Hyatt)
        Instead of testing each move
        for legality we could simply go ahead and recurse into each node and
        if there is a king capture in one of the succeeding nodes we jump back
        and dismiss this move.

v 0.4 (plan)
================================================================================
- TODO:
    - LFP. LFR, ...
    - Extended Evaluation
    - EvalCache
    - Aspiration Window Search

v 0.3 (in progress)
================================================================================
- TODO:
    - Testing and Debugging
- DONE:
    - RFP, NMP, RAZOR
    - IID (with TT - needs to be revisited)
    - PV from TT and PV Move Sorting
    - PVS
    - MDP, MPP

v 0.2 (done)
================================================================================
- DONE:
    - Testing & Debugging
    - TT in qsearch
    - TT (not in qsearch)
    - Killer moves
    - Quiescence Search
    - Templated Search for Root, non-root and qsearch

v 0.1 (done)
================================================================================
- DONE:
    - AlphaBeta search
    - Configuration Framework for Search and Evaluation
    - Testing & Debugging
    - check draw by repetition or 50moves rule
    - Complete UCI Protocol
    - TreeSize Test
    - Pondering
    - Basic search (Minimax, no TT, no KillerMoves, etc.)
    - Basic evaluation
    - Search Modes (incl. basic time management for time based search)
    - Perft traversing (minimax with counting)
    - Logging
    - UCI Protocol: Basics
    - PERFT
        - Foundation Datatypes such as Bitboards, Move etc.
        - Position
        - basic MoveGen





