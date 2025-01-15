FROM gitpod/workspace-full::2025-01-15-08-55-28

RUN sudo apt-get update && sudo apt-get install -y libsdl2-dev libsdl2-2.0-0
