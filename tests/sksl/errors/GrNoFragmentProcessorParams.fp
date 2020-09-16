in fragmentProcessor child;

half4 helper(fragmentProcessor fp) { return sample(fp); }

void main() {
    sk_OutColor = helper(child);
}
