in fragmentProcessor fp;

void main() {
     sk_OutColor = sample(fp, sk_InColor, sk_FragCoord.xy);
}