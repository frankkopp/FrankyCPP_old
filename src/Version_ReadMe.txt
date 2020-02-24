v 0.x (plan)
================================================================================
- TODO:
    - better prunings
    - IDEA: (credit to Robert Hyatt)
        Instead of testing each move
        for legality we could simply go ahead and recurse into each node and
        if there is a king capture in one of the succeeding nodes we jump back
        and dismiss this move.

v 0.6 (in progress)
================================================================================
- TODO:
    - Opening Book Cache
    - make prunings work
    - ATACKS / SEE
    - Configure for play strength
- DONE:

v 0.5 (done)
================================================================================
- DONE:
    - Testing
    - UCI Options
    - Simple Opening Book
    - TestSuites
    - Beta Cut Movenumber Counter
    - Aspiration Window Search
    - RAZOR, LFP. LFR, etc.

v 0.4 (done)
================================================================================
- DONE:
    - Compile on Windows with Cygwin and VS and on Mac with Clion. Xcode not yet
    - Debug Search - blunders (IID removed for now)
    - Debug and harden play with UCI GUI  XBoard, Arena, etc.
    - See logfile for errors
    - New Game - clear hash
    - FASTER LINK TIMES
    - Compile under Cygwin on PC
    - move test src one up
    - Put Position history variables in a struct for better locality
    - Use CLANG compiler warnings
    - Extended Evaluation
    - Eval pawn cache

v 0.3 (done)
================================================================================
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





