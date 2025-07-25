from aios_core import AIOS
from metalifex_engine import MetalifeX

if __name__ == "__main__":
    print("🔵 MetalifeX AIOS tizimi ishga tushmoqda...")

    aios = AIOS()
    aios.init_system()

    metalifex = MetalifeX(aios)
    metalifex.load_world()

    print("✅ Hammasi tayyor! Dunyo yaratishga kirishing!")
    while True:
        action = input("🕹️ Harakatni kiriting: ")
        metalifex.process_action(action)