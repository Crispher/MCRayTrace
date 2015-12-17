#include "Sampler.h"

#pragma region BASE_SAMPLER

Sampler3D::Sampler3D() {
	generatePreSamples_Diffuse(737797);
	generatePreSamples_Specular(717977, 1000);
}

std::vector<Sample2D> Sampler2D::sampleTriangle(int n) {
	std::vector<Sample2D> samples2D = sample_UnitSquare_Uniform(n);
	for (int i = 0; i < samples2D.size(); i++) {
		if (samples2D[i].u + samples2D[i].v > 1) {
			Real temp = samples2D[i].u;
			samples2D[i].u = 1.0 - samples2D[i].v;
			samples2D[i].v = 1.0 - temp;
		}
	}
	return samples2D;
}

Sample2D Sampler3D::sampleUnitSquare_Uniform() {
	return Sample2D(uniform_01(gen), uniform_01(gen));
}

Real Sampler3D::getUniform_01() {
	return uniform_01(gen);
}

void Sampler3D::generatePreSamples_Diffuse(int numSamples) {
	presamples_Diffuse_Single.cursor = 0;
	presamples_Diffuse_Single.elementCount = numSamples;
	for (int i = 0; i < numSamples; i++) {
		Sample2D sample2D = sampleUnitSquare_Uniform();
		Real sq = sqrt(sample2D.u);
		presamples_Diffuse_Single.presamples.push_back(
			PreSample3D(
			sq * cos(2 * Constants::Pi * sample2D.v),
			sq * sin(2 * Constants::Pi * sample2D.v),
			sqrt(1 - sample2D.u),
			1));
	}
}

void Sampler3D::generatePreSamples_Specular(int numSamples, int N) {
	if (presamples_Specular_Single.find(N) != presamples_Specular_Single.end() || N < 0) {
		return;
	}
	PreSamplePool3D psp;
	psp.cursor = 0;
	psp.elementCount = numSamples;
	for (int i = 0; i < numSamples; i++) {
		Sample2D sample2D = sampleUnitSquare_Uniform();
		Real cosphi;
		if (N >= 1000) {
			cosphi = 1;
		}
		else {
			cosphi = pow(1 - sample2D.u, 1. / N);
		}
		Real sinphi = sqrt(1 - cosphi * cosphi);
		Real theta = 2 * Constants::Pi * sample2D.v;
		psp.presamples.push_back(PreSample3D(
			sinphi * cos(theta),
			sinphi * sin(theta),
			cosphi,
			1));
	}
	presamples_Specular_Single.insert(std::pair<int, PreSamplePool3D>(N, psp));
}

Vector3R Sampler3D::sample_Diffuse_P(const Vector3R &normal) {
	Vector3R dummy(uniform_01(gen), uniform_01(gen), uniform_01(gen));
	Vector3R x = dummy.cross(normal).normalized();
	Vector3R y = normal.cross(x);
	PreSample3D &ps = presamples_Diffuse_Single.presamples[presamples_Diffuse_Single.cursor];
	++presamples_Diffuse_Single.cursor %= presamples_Diffuse_Single.elementCount;
	return (ps.x) * x + ps.y * y + ps.z * normal;
}

Vector3R Sampler3D::sample_Specular_P(const Vector3R &out, const Vector3R &normal, int N) {
	Vector3R dummy(uniform_01(gen), uniform_01(gen), uniform_01(gen));
	Vector3R x = dummy.cross(normal).normalized();
	Vector3R y = normal.cross(x);
	PreSamplePool3D &psp = presamples_Specular_Single[N];
	PreSample3D &ps = psp.presamples[psp.cursor];
	++psp.cursor %= psp.elementCount;
	return (ps.x) * x + ps.y * y + ps.z * out;
}

RaySample Sampler3D::sample(const Vector3R &in, const Vector3R &normal, const MaterialPtr &mPtr) {
	RaySample ans;
	bool lookFromOutside = in.dot(normal) < 0;
	if (!mPtr->isTr()) {
		// from outside, reflective diffuse and specular light
		Real wd = (mPtr->Kd[3]) / (mPtr->Kd[3] + mPtr->Ks[3]);
		if (uniform_01(gen) - Limit::Epsilon < wd) {
			ans.type = RE_D;
			ans.weight = wd;
			ans.v = sample_Diffuse_P(normal);
			return ans;
		}
		else {
			ans.type = RE_S;
			ans.weight = 1 - wd;
			ans.v = sample_Specular_P(reflect(in, normal), normal, mPtr->Ns);
			return ans;
		}
	}
	else {
		if (lookFromOutside) {
			// look from outside, transparent material
			Real total = (mPtr->Kd[3] + mPtr->Ks[3] + mPtr->Td[3] + mPtr->Ts[3]);
			Real w, t = 0;
			Real rand = uniform_01(gen);
			if (rand - Limit::Epsilon < (t += (w = mPtr->Kd[3] / total))) {
				ans.type = RE_D;
				ans.weight = w;
				ans.v = sample_Diffuse_P(normal);
				return ans;
			}
			else if (rand + Limit::Epsilon < (t += (w = mPtr->Ks[3] / total))) {
				ans.type = RE_S;
				ans.weight = w;
				ans.v = sample_Specular_P(reflect(in, normal), normal, mPtr->Ns);
				return ans;
			}
			else if (rand - Limit::Epsilon< (t += (w = mPtr->Td[3] / total))) {
				ans.type = TR_D;
				ans.weight = w;
				ans.v = sample_Diffuse_P(-normal);
				return ans;
			}
			else {
				ans.type = TR_S;
				ans.weight = 1 - t;
				ans.v = sample_Specular_P(refract(in, normal, mPtr->n), normal, mPtr->Nst);
				return ans;
			}
		}
		else {
			// look from inside, transparent material
			// to do: fresnel
			Real wd = (mPtr->Td[3]) / (mPtr->Td[3] + mPtr->Ts[3]);
			if (uniform_01(gen) - Limit::Epsilon < wd) {
				ans.type = TR_D;
				ans.weight = wd;
				ans.v = sample_Diffuse_P(normal);
				return ans;
			}
			else {
				ans.type = TR_S;
				ans.weight = 1 - wd;
				ans.v = sample_Specular_P(refract(in, normal, mPtr->n), normal, mPtr->Nst);
				return ans;
			}
		}
	}
}

#pragma endregion

#pragma region STRATIFIED_SAMPLER

std::vector<Sample2D> StratifiedSampler::sample_UnitSquare_Uniform(int sampleSize) {
	Real step = 1.0 / sampleSize;
	std::vector<Sample2D> samples2R(sampleSize * sampleSize);
	for (int i = 0; i < sampleSize; i++) {
		for (int j = 0; j < sampleSize; j++) {
			samples2R[i * sampleSize + j] = Sample2D((i + uniform_01(gen)) * step, (j + uniform_01(gen)) * step);
		}
	}
	return samples2R;
}

#pragma endregion

#pragma region LATINCUBE_SAMPLER

std::vector<Sample2D> LatinCubeSampler::sample_UnitSquare_Uniform(int sampleSize) {
	Real step = 1.0 / sampleSize;
	std::vector<Sample2D> samples2R(sampleSize);
	std::vector<int> permutation(sampleSize);
	for (int i = 0; i < sampleSize; i++) {
		samples2R[i] = Sample2D(uniform_01(gen), uniform_01(gen));
		permutation[i] = i;
	}
	std::shuffle(permutation.begin(), permutation.end(), std::default_random_engine());
	for (int i = 0; i < sampleSize; i++) {
		samples2R[i].u = (samples2R[i].u + permutation[i]) * step;
		samples2R[i].v = (i + samples2R[i].v) * step;
	}
	return samples2R;
}

#pragma endregion