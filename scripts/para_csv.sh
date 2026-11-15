#!/bin/bash
# Converte resultados/*.out em CSVs para o Excel (separador ';', decimal com ponto).
#   scripts/para_csv.sh [pasta_resultados=resultados] [pasta_saida=csv]
R=${1:-resultados}; O=${2:-csv}; mkdir -p "$O"
echo "arquivo;versao;P;yield;rep;n;delta;tempo_s;verificacao" > "$O/paralelo.csv"
echo "arquivo;modo;n;delta_ou_P;rep;tempo_s;verificacao;folhas;menor_folha;maior_folha" > "$O/sequencial.csv"
echo "arquivo;versao;P;yield;rep;rank;nivel;tam;folha;t_bubble_s;t_merge_s;t_espera_s" > "$O/por_processo.csv"
for f in "$R"/*.out; do
  awk -v F="$(basename "$f")" '
    function kv(s,  a,i,p,n){ n=split(s,a," "); for(i=1;i<=n;i++){ p=index(a[i],"="); if(p) v[substr(a[i],1,p-1)]=substr(a[i],p+1) } }
    /^# .*versao=/ { rep=0; kv($0); y=v["yield"]; if(y=="") y="nd" }
    /^SEQ/    { kv($0); if(v["modo"]=="balanceada") { id=v["P"]; nm="balanceada"; fo=v["folhas"];me=v["menor"];ma=v["maior"] }
                else { id=v["delta"]; nm=(v["delta"]==v["n"])?"T1":"delta"; fo="";me="";ma="" }
                sr[nm id]++; print F";"nm";"v["n"]";"id";"sr[nm id]";"v["tempo"]";"v["verificacao"]";"fo";"me";"ma >> "'"$O"'/sequencial.csv" }
    /^RESULT/ { kv($0); rep++; ver=v["versao"]; P=v["P"]; yy=(ver=="original")?y:"-";
                print F";"ver";"P";"yy";"rep";"v["n"]";"v["delta"]";"v["tempo"]";"v["verificacao"] >> "'"$O"'/paralelo.csv" }
    /^STAT/   { kv($0); print F";"ver";"P";"yy";"rep";"v["rank"]";"v["nivel"]";"v["tam"]";"v["folha"]";"v["t_bubble"]";"v["t_merge"]";"v["t_espera"] >> "'"$O"'/por_processo.csv" }
  ' "$f"
done
wc -l "$O"/*.csv; echo "execuções com ERRO:"; grep -c ';ERRO' "$O"/paralelo.csv "$O"/sequencial.csv
