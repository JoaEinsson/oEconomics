import csv
import sys
import os
import io

# Força o terminal a usar UTF-8 no Windows para suportar os Emojis
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

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
    
    # Extract phenotype columns
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
    total_tools_val = [float(row.get('Total_Tools', 0)) for row in data]
    total_gems = [float(row.get('Total_Gems', 0)) for row in data]
    
    active_tools = [int(row.get('Active_Tools', 0)) for row in data]
    tools_crafted = [int(row.get('Tools_Crafted', 0)) for row in data]
    
    deaths_starve = [int(row.get('Deaths_Starvation', 0)) for row in data]
    deaths_age = [int(row.get('Deaths_OldAge', 0)) for row in data]
    deaths_combat = [int(row.get('Deaths_Combat', 0)) for row in data]

    print("==================================================")
    print(" RELATÓRIO EXECUTIVO SOCIOECONÔMICO (OECONOMICS)  ")
    print("==================================================")
    print(f"Duração da Simulação: {total_ticks} Ticks ({ticks[0]} -> {ticks[-1]})")
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

    # === SEÇÃO EXPANDIDA: BENS DE CAPITAL ===
    print("\n[ 🛠️ Bens de Capital Físico e Produção ]")
    print(f"- Ferramentas Ativas no Último Tick: {active_tools[-1]}")
    print(f"- Total de Ferramentas Fabricadas (Acumulado): {tools_crafted[-1]}")
    print(f"- Fazendas/Estruturas Agrícolas no Último Tick: {farms[-1]}")
    
    peak_tools = max(active_tools)
    peak_tools_tick = ticks[active_tools.index(peak_tools)]
    print(f"- Pico de Ferramentas Ativas: {peak_tools} (Tick {peak_tools_tick})")
    
    peak_farms = max(farms)
    peak_farms_tick = ticks[farms.index(peak_farms)]
    print(f"- Pico de Fazendas: {peak_farms} (Tick {peak_farms_tick})")
    
    # Densidade de ferramentas per capita
    if pops[-1] > 0:
        tools_per_capita = active_tools[-1] / pops[-1]
        farms_per_capita = farms[-1] / pops[-1]
        print(f"- Ferramentas Per Capita (Final): {tools_per_capita:.3f}")
        print(f"- Fazendas Per Capita (Final): {farms_per_capita:.3f}")
    
    # Taxa de sobrevivência de ferramentas (ativas vs fabricadas)
    if tools_crafted[-1] > 0:
        survival_rate = active_tools[-1] / tools_crafted[-1] * 100
        print(f"- Taxa de Sobrevivência de Ferramentas: {survival_rate:.1f}% ({active_tools[-1]}/{tools_crafted[-1]})")
        breakage = tools_crafted[-1] - active_tools[-1]
        print(f"- Ferramentas Quebradas/Destruídas: {breakage}")
    
    # Riqueza material total
    print(f"\n- Riqueza Material Total (Último Tick):")
    print(f"  • PIB Calórico Retido: {food[-1]:.0f} kcal")
    print(f"  • Dureza Total (Hardness): {total_tools_val[-1]:.0f}")
    print(f"  • Gemas/Estética Total: {total_gems[-1]:.0f}")

    print("\n[ 📉 Macroeconomia e Desigualdade ]")
    print(f"- Índice de Gini Final: {gini[-1]:.3f} (0=Igualitário, 1=Estratificado)")
    print(f"- Ordens de Mercado no último tick: {orders[-1]}")
    print(f"- Confiança Máxima no Grafo (Max Trust): {max_trust[-1]:.1f}")

    print("\n[ 💀 Causa Mortis Acumulada ]")
    total_deaths = deaths_starve[-1] + deaths_age[-1] + deaths_combat[-1]
    print(f"- Total de Mortes: {total_deaths}")
    print(f"- Morte por Inanição (Fome): {deaths_starve[-1]}")
    print(f"- Morte por Velhice (Entropia L10): {deaths_age[-1]}")
    print(f"- Morte em Combate (Assalto): {deaths_combat[-1]}")
    if total_deaths > 0:
        print(f"- Distribuição: Fome {deaths_starve[-1]/total_deaths*100:.0f}% | Velhice {deaths_age[-1]/total_deaths*100:.0f}% | Combate {deaths_combat[-1]/total_deaths*100:.0f}%")

    # === TIMELINE EXPANDIDA ===
    print("\n### Timeline (A cada 20% do tempo)")
    print("| Tick | Pop | Avg E | Farms | Atv.Tools | Crafted | Gini | Trust | Food | Deaths |")
    print("|---|---|---|---|---|---|---|---|---|---|")
    
    step = max(1, total_ticks // 5)
    for i in range(0, total_ticks, step):
        idx = min(i, total_ticks - 1)
        total_d = deaths_starve[idx] + deaths_age[idx] + deaths_combat[idx]
        print(f"| {ticks[idx]} | {pops[idx]} | {energies[idx]:.0f} | {farms[idx]} | {active_tools[idx]} | {tools_crafted[idx]} | {gini[idx]:.2f} | {max_trust[idx]:.1f} | {food[idx]:.0f} | {total_d} |")
        
    # Last tick
    total_d = deaths_starve[-1] + deaths_age[-1] + deaths_combat[-1]
    print(f"| {ticks[-1]} | {pops[-1]} | {energies[-1]:.0f} | {farms[-1]} | {active_tools[-1]} | {tools_crafted[-1]} | {gini[-1]:.2f} | {max_trust[-1]:.1f} | {food[-1]:.0f} | {total_d} |")

if __name__ == '__main__':
    filepath = 'telemetry.csv'
    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    elif not os.path.exists(filepath) and os.path.exists('build/Debug/telemetry.csv'):
        filepath = 'build/Debug/telemetry.csv'
    
    analyze(filepath)
