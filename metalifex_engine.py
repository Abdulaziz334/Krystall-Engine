import json

class MetalifeX:
    def __init__(self, aios):
        self.aios = aios
        self.world = {}

    def load_world(self):
        try:
            with open("world_structure.json", "r") as f:
                self.world = json.load(f)
            print("🌍 MetalifeX dunyosi yuklandi.")
        except:
            print("❌ Dunyo yuklanmadi. Iltimos, world_structure.json faylini tekshiring.")

    def process_action(self, action):
        print(f"🤖 AI harakatni analiz qilmoqda: '{action}'")
        if "bor" in action:
            print("🚶‍♂️ Siz harakat qildingiz.")
        elif "qur" in action:
            print("🏗️ Qurilish boshlandi.")
        elif "sotib ol" in action:
            print("💰 Xarid amalga oshirildi.")
        else:
            print("❓ Noma'lum harakat.")