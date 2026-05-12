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
    
    # Extract columns
    ticks = [int(row['Tick']) for row in data]
    pops = [int(row['Pop_Alive']) for row in data]
    energies = [float(row['Avg_Energy']) for row in data]
    farms = [int(row['Farms_Count']) for row in data]
    ages = [float(row['Avg_Age']) for row in data]
    metab = [float(row['Avg_Metabolism']) for row in data]

    print("### Resumo de 2000 Ticks")
    print(f"- **População Inicial:** {pops[0]}")
    print(f"- **População Final:** {pops[-1]}")
    print(f"- **Pico Populacional:** {max(pops)} (Tick {ticks[pops.index(max(pops))]})")
    print(f"- **Mínimo Populacional:** {min(pops)} (Tick {ticks[pops.index(min(pops))]})")
    
    print("\n### Energia e Sobrevivência")
    print(f"- **Energia Inicial:** {energies[0]:.1f}")
    print(f"- **Energia Final:** {energies[-1]:.1f}")
    
    print("\n### Tabela Temporal (A cada 400 ticks)")
    print("| Tick | População | Energia Média | Estruturas | Metabolismo Médio | Idade Média |")
    print("|---|---|---|---|---|---|")
    
    step = max(1, total_ticks // 5)
    for i in range(0, total_ticks, step):
        idx = min(i, total_ticks - 1)
        print(f"| {ticks[idx]} | {pops[idx]} | {energies[idx]:.1f} | {farms[idx]} | {metab[idx]:.3f} | {ages[idx]:.1f} |")
        
    # Last tick
    print(f"| {ticks[-1]} | {pops[-1]} | {energies[-1]:.1f} | {farms[-1]} | {metab[-1]:.3f} | {ages[-1]:.1f} |")

if __name__ == '__main__':
    analyze('build/Debug/telemetry.csv')
