# Futuras Implementações - OECONOMICS

Ideias e mecânicas emergentes alinhadas à Constituição do Motor (Zero OOP, Conservação Termodinâmica, Lógica Tensorial).

## Eixo 1: Aprofundamento Econômico e Material (Produção e Valor)

- [ X ] **Bens de Capital Físico (Ferramentas que geram Alavancagem):**
  - **Mecânica:** Introduzir Itens com alto índice de `Hardness` e `Information` (Ferramentas).
  - **Efeito:** Se mantidos no inventário durante as intenções `Farm` ou `CraftItem`, o gasto basal de energia cai pela metade ou o rendimento (yield) dobra.
  - **Entropia:** Cada uso consome `integrity` da ferramenta, gerando demanda cíclica para forjadores.

- [ X ] **O Surgimento do Crédito e "Violência Legal" (Contratos Imperfeitos):**
  - **Mecânica:** Agentes com alto nível de `Trust` podem trocar matéria real por uma "Nota Promissória" (um Item físico cujo tensor carrega o ID do devedor).
  - **Consequência:** Se o devedor não honrar a promessa no futuro, a aresta do `TrustGraph` despenca para negativo severo, forçando a ativação do meme `Aggro` (Guerra por Quebra de Contrato).

## Eixo 2: Dinâmicas Ambientais e Sobrevivência (Termodinâmica)

- [ ] **Clima, Estações e a Necessidade de Abrigo:**
  - **Mecânica:** Adicionar um ciclo senoidal global de Temperatura. Temperaturas extremas aumentam drasticamente o `basal_cost`. (comentário do dev: acho melhor isso ser na geração da "comida" em sí também, mas sim a temperatura também deve ter impacto nas entidades de alguma forma)
  - **Solução:** Agentes devem craftar e equipar itens com propriedade térmica alta (Roupas/Abrigos). Estar perto de Fogueiras (Itens ancorados) mitiga a perda de energia térmica, gerando o surgimento emergente de "Cidades".

- [ ] **Entropia Biológica (Doenças e Parasitas Vetoriais):**
  - **Mecânica:** Alta densidade populacional (Mercados) cria risco de repassar uma "Doença" (um vetor indesejado) via colisão no grid.
  - **Efeito:** A doença impõe um dreno passivo severo na energia. A cura exige o consumo de Itens cujo tensor neutralize o vetor da doença (Ervas Medicinais).

## Eixo 3: Cultura, Memética e Política Emergente

- [ ] **Tribalismo Estigmergico (Ressonância Ideológica):**
  - **Mecânica:** O fenótipo/memes de agentes próximos irradiam e se alinham. A distância de cosseno entre vetores dita a tolerância de convivência.
  - **Consequência:** Surgimento natural de Guerras Culturais/Tribais por território quando a distância vetorial for grande e a energia/fome estiver crítica.

- [ ] **Marcadores Territoriais (Sinalização de Propriedade):**
  - **Mecânica:** Agentes transformam recursos num "Totem" (Item com flag `anchored=true` e alto tensor de `Artistry`).
  - **Efeito:** Irradia o ID do criador. Estranhos (sem `Trust`) que entram no raio do Totem sofrem penalidade de `basal_cost` por "estresse psicológico", a menos que destruam o marcador ou se aliem ao dono.
