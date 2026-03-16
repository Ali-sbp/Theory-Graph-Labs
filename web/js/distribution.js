window.FarryDistribution = class {
  constructor(p) {
    this.p = Math.max(0.001, Math.min(0.999, p));
    this.logq = Math.log(1.0 - this.p);
  }
  generate() {
    let u = Math.random();
    if (u <= 0) u = 1e-12;
    if (u >= 1) u = 1 - 1e-12;
    if (this.logq >= 0) return 1;
    const val = Math.log(1.0 - u) / this.logq;
    if (!isFinite(val) || val <= 0) return 1;
    return Math.max(1, Math.ceil(val));
  }
};
