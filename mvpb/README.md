# Mandelbrot - Infraestrutura de Software (Implementação 2)

Gera imagens do conjunto de Mandelbrot em quatro implementações diferentes: Serial, OpenMP, Pthreads 1 (divisão estática de linhas) e Pthreads 2 (fila dinâmica de trabalho com otimização de cardioide/bulbo).

## Compilar

```bash
make clean
make
```

## Executar

```bash
./mandelbrot [largura] [altura] [max_iteracoes] [num_threads]
```

Exemplo:

```bash
./mandelbrot 400 400 100 4
```

## Saídas geradas

- `mandelbrot_mvpb_serial.pgm`
- `mandelbrot_mvpb_openmp.pgm`
- `mandelbrot_mvpb_pthreads1.pgm`
- `mandelbrot_mvpb_pthreads2.pgm`
- `times.txt` (tempo de execução de cada implementação)
- `falha.txt` (mensagens de erro, se houver)

## Limpar arquivos gerados

```bash
make clean
```

Detalhes de implementação, decisões técnicas e evidências de teste estão no relatório (`mvpb.pdf`).
