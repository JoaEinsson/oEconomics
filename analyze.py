import csv
import sys

def analyze(filepath):
    try:
        with open(filepath, 'r') as f:
            reader = csv.DictReader(f)
            data = list(reader)
    except Exception as e:
        print(f"Error: {e}")
        return

    if not data:
        print("No data in telemetry.csv")
        return

    total_ticks = len(data)
    
    # Extract basic columns
    ticks = [int(row['Tick']) for row in data]
    pops = [int(row['Pop_Alive']) for row in data]
    energies = [float(row['Avg_Energy']) for row in data]
    farms = [int(row['Farms_Count']) for row in data]
    ages = [float(row['Avg_Age']) for row in data]
    metab = [float(row['Avg_Metabolism']) for row in data]
    
    # Extract new columns
    panic_th = [float(row['Avg_Panic_Th']) for row in data]
    min_repro = [float(row['Avg_Min_Repro']) for row in data]
    attack_th = [float(row['Avg_Attack_Th']) for row in data]
    barter_desire = [float(row['Avg_Barter_Desire']) for row in data]
    knowledge = [float(row['Avg_Knowledge']) for row in data]
    intel = [float(row['Avg_Intelligence']) for row in data]
    artistry = [float(row['Avg_Artistry']) for row in data]
    gini = [float(row['Gini_Index']) for row in data]
    orders = [int(row['Market_Orders']) for row in data]
    max_trust = [float(row['Max_Trust']) for row in data]
    food = [float(row['Total_Food']) for row in data]
    
    deaths_starve = [int(row.get('Deaths_Starvation', 0)) for row in data]
    deaths_age = [int(row.get('Deaths_OldAge', 0)) for row in data]
    deaths_combat = [int(row.get('Deaths_Combat', 0)) for row in data]

    print("==================================================")
    print(" RELATÓRIO EXECUTIVO SOCIOECONÔMICO (OECONOMICS)  ")
    print("==================================================")
    print(f"Duração da Simulação: {total_ticks} Ticks")
    print(f"- **População Inicial:** {pops[0]} -> **População Final:** {pops[-1]}")
    print(f"- **Pico Populacional:** {max(pops)} (Tick {ticks[pops.index(max(pops))]})")
    
    print("\n[ 🧬 Evolução Cognitiva (Fenótipos) ]")
    print(f"- Tolerância ao Pânico (Fome): {panic_th[0]:.1f} -> {panic_th[-1]:.1f}")
    print(f"- Idade Mínima de Reprodução: {min_repro[0]:.1f} -> {min_repro[-1]:.1f}")
    print(f"- Agressividade Básica (Banditismo): {attack_th[0]:.1f} -> {attack_th[-1]:.1f}")
    print(f"- Desejo de Comércio (Mercantilismo): {barter_desire[0]:.1f} -> {barter_desire[-1]:.1f}")
    
    print("\n[ 🧠 Evolução do Conhecimento e Arte ]")
    print(f"- Experiência Média (XP Acumulado): {knowledge[0]:.2f} -> {knowledge[-1]:.2f}")
    print(f"- Taxa Genética de Aprendizado: {intel[0]:.2f} -> {intel[-1]:.2f}")
    print(f"- Taxa Genética de Apreciação Estética: {artistry[0]:.2f} -> {artistry[-1]:.2f}")

    print("\n[ 📉 Macroeconomia e Desigualdade ]")
    print(f"- Índice de Gini Final: {gini[-1]:.3f} (0=Igualitário, 1=Estratificado)")
    print(f"- Ordens de Mercado no último tick: {orders[-1]}")
    print(f"- Confiança Máxima no Grafo (Max Trust): {max_trust[-1]:.1f}")
    print(f"- PIB Calórico Retido: {food[-1]:.0f} kcal")

    print("\n[ 💀 Causa Mortis Acumulada ]")
    print(f"- Morte por Inanição (Fome): {deaths_starve[-1]}")
    print(f"- Morte por Velhice (Entropia L10): {deaths_age[-1]}")
    print(f"- Morte em Combate (Assalto): {deaths_combat[-1]}")

    print("\n### Timeline (A cada 20% do tempo)")
    print("| Tick | População | Gini  | Trust Max | Orders | Pânico Média | Causa M. Fome |")
    print("|---|---|---|---|---|---|---|")
    
    step = max(1, total_ticks // 5)
    for i in range(0, total_ticks, step):
        idx = min(i, total_ticks - 1)
        print(f"| {ticks[idx]} | {pops[idx]} | {gini[idx]:.2f} | {max_trust[idx]:.1f} | {orders[idx]} | {panic_th[idx]:.1f} | {deaths_starve[idx]} |")
        
    # Last tick
    print(f"| {ticks[-1]} | {pops[-1]} | {gini[-1]:.2f} | {max_trust[-1]:.1f} | {orders[-1]} | {panic_th[-1]:.1f} | {deaths_starve[-1]} |")

if __name__ == '__main__':
    # Try looking in root or build/Debug depending on how it's executed
    try:
        analyze('telemetry.csv')
    except:
        analyze('build/Debug/telemetry.csv')
