in fragmentProcessor child;

half4 helper(fragmentProcessor fp) { return sample(fp); }

half4 main() {
    return helper(child);
}
